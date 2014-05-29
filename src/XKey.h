#ifndef XKEY_DATA_H
#define XKEY_DATA_H

#include <string>
#include <deque>
#include <memory>

namespace Json {
	class Value;
}

namespace XKey {

class Folder;

/**
 * @brief A single key entry
 */
class Entry
{
public:
	inline Entry () { }
	inline Entry (const std::string &title, const std::string &user, const std::string &url, const std::string &pwd, const std::string &email, const std::string &comment);
	
	inline const std::string&			title() const { return _title; }
	inline const std::string&			username() const { return _username; }
	inline const std::string&			url() const { return _url; }
	inline const std::string&			password() const { return _password; }
	inline const std::string&			email() const { return _email; }
	inline const std::string&			comment() const { return _comment; }
private:
	std::string _title;
	std::string _username;
	std::string _url;
	std::string _password;
	std::string _email;
	std::string _comment;
};

typedef std::unique_ptr<Folder> RootFolder_Ptr;

/**
 * @brief Key folder containing subfolders and any amount of keys
 */
class Folder
{
public:
	Folder ();
	Folder (const std::string &name, Folder *parent);
	
	void operator= (Folder &&);
	
	inline const std::string&			name() const { return _name; }
	inline Folder*					parent() const { return _parent; }
	
	void 							setName (const std::string &name);
	
	std::string						fullPath() const;
	
	void 							addEntry (Entry entry);
	Entry &						getEntryAt (int index);
	void							removeEntry (int index);
	Folder *						createSubfolder (const std::string &name);
	void							removeSubfolder (int index);
	
	Folder * 						getSubfolder (const std::string &name);
	const Folder * 					getSubfolder (const std::string &name) const;
	
	inline const std::deque<Folder>&	subfolders () const { return _subfolders; }
	inline const std::deque<Entry>&	entries () const { return _entries; }
	
	inline std::deque<Folder>&		subfolders () { return _subfolders; }
	inline std::deque<Entry>&		entries () { return _entries; }
	
	int 							row() const;
private:
	std::string _name;
	std::deque<Folder> _subfolders;
	std::deque<Entry> _entries;
	Folder *_parent;
	// Disallow copying
	Folder (const Folder &) = delete;
	Folder &operator= (const Folder &) = delete;

	friend RootFolder_Ptr createRootFolder ();
};

/**
 * @brief Create a new root folder for a folder hierarchy
 * 
 * This method is necessary, because folders must NOT be allocated on the stack.
 */
inline RootFolder_Ptr createRootFolder () {
	return RootFolder_Ptr (new Folder());
}

/**
 * @brief Result structure for searching in a key-tree structure
 * 
 * An object of this class gets returned by @continueSearch or @startSearch.\n
 * It serves as an iterator into the key-tree and can be given as a parameter to @continueSearch to return the next result.\n
 * \n
 * You must not modify the tree-structure while performing a search, or the result is undefined.
 */
struct SearchResult
{
	inline SearchResult () : _match(0), _lastFolder(0), _lastIndex(-1) { }
	inline SearchResult (const Entry *e, const Folder *f, int ind) : _match(e), _lastFolder(f), _lastIndex(ind) { }
	
	inline const Entry *match () const {
		return _match;
	}
	
	/// Check if the SearchResult contains a valid result match
	inline bool hasMatch () const { return _match != 0; }
	
	inline const Folder *parentFolder () const { return _lastFolder; }
	
	inline int index () const { return _lastIndex; }
	
	const Entry *_match;
	const Folder *_lastFolder;
	int _lastIndex;
};

/**
 * @brief Search for entries in a folder hierarchy incrementally
 * @param searchString The string to search for. If it contains spaces, all words will be searched for independently
 * @param rootFolder Root-node of the hierarchy
 * @return Empty SearchResult-set if no matching entry was found
 */
SearchResult startSearch (const std::string &searchString, const Folder *rootFolder);

/**
 * @brief Continue a search that was previously started with @startSearch
 * @param searchString String to search for. Should be the same as the one for @startSearch
 * @param lastResult reference to the last result of either @startSearch or @continueSearch
 * @return Next result, ort empty SearchResult if no matching entry was found
 */
SearchResult continueSearch (const std::string &searchString, const SearchResult &lastResult);

/**
 * @brief Search folder by name in a folder hierarchy.
 * @param root The root object of the hierarchy to search
 * @param search_path Search path to follow, must begin with a slash. Example: "/test1/Folder 2"
 * @return NULL if the folder was not found.
 */
const Folder *getFolderByPath (const Folder *root, const std::string &search_path);

/**
 * @brief Move a folder in the hierarchy
 * @param oldFolder Folder to be moved. Shall not be the root folder
 * @param newParent Parent folder to insert the folder to
 * @param newPosition the new element is inserted BEFORE this position in the list of the new parents subfolder. 
 */
Folder *moveFolder (Folder *oldFolder, Folder *newParent, int newPosition);

/**
 * @brief Reader to parse XKey structures from cleartext streams
 */
class Parser
{
public:
	bool 					readFile (std::istream &in, Folder *root);

	const std::string&		error () const;
private:
	void 					parse_folder (const Json::Value &folder, Folder *parent);
	void 					parse_folder_list (const Json::Value &item, Folder *parent);
	void 					parse_key (const Json::Value &key_entry, Folder *parent);
	
	std::string errorMsg;
};

/**
 * @brief Writer to write XKey structures to cleartext streams
 */
class Writer
{
public:
	bool 					writeFile (std::ostream &out, const Folder &root, bool write_formatted = false);

	const std::string&		error () const;

	/**
	 * @brief Assert correct file permissions
	 * @param filename full pathname
	 * @param correctReadPermissions if not NULL, check if read permissions are set correctly (only owner can read)
	 * @param canWrite if not NULL, containts whether the application can write to the file
	 * @return true iff file exists
	 */
	static bool				checkFilePermissions (const std::string &filename, bool *correctReadPermissions, bool *canWrite = nullptr);

	/// Make a file readable and writable only by it's owner
	static void				setRestrictiveFilePermissions (const std::string &filename);
private:
	void 					serialize_folder (Json::Value &parent, const Folder &folder);
	
	std::string errorMsg;
};


// Impl

Entry::Entry (const std::string &title, const std::string &user, const std::string &url, const std::string &pwd, const std::string &email, const std::string &comment) 
	: _title(title), _username (user), _url(url), _password (pwd), _email(email), _comment (comment)
{}

}

#endif
