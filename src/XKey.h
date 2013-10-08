#ifndef XKEY_H
#define XKEY_H

#include <string>
#include <deque>
#include <memory>

namespace Json {
	class Value;
}

namespace XKey {

class Folder;

class Entry
{
public:
	inline Entry () { }
	inline Entry (const std::string &title, const std::string &user, const std::string &url, const std::string &pwd, const std::string &comment);
	
	inline const std::string&			title() const { return _title; }
	inline const std::string&			username() const { return _username; }
	inline const std::string&			url() const { return _url; }
	inline const std::string&			password() const { return _password; }
	inline const std::string&			comment() const { return _comment; }
private:
	std::string _title;
	std::string _username;
	std::string _url;
	std::string _password;
	std::string _comment;
};

typedef std::unique_ptr<Folder> RootFolder_Ptr;

class Folder
{
public:
	Folder ();
	Folder (const std::string &name, Folder *parent);
	
	void operator= (Folder &&);
	
	inline const std::string&			name() const { return _name; }
	inline Folder*						parent() const { return _parent; }
	
	void 								setName (const std::string &name);
	
	std::string							fullPath() const;
	
	void 								addEntry (Entry entry);
	void								removeEntry (int index);
	Folder *							createSubfolder (const std::string &name);
	void								removeSubfolder (int index);
	
	Folder * 							getSubfolder (const std::string &name);
	const Folder * 						getSubfolder (const std::string &name) const;
	
	inline const std::deque<Folder>&	subfolders () const { return _subfolders; }
	inline const std::deque<Entry>&		entries () const { return _entries; }
	
	inline std::deque<Folder>&			subfolders () { return _subfolders; }
	inline std::deque<Entry>&			entries () { return _entries; }
	
	 int 								row() const;
private:
	std::string _name;
	std::deque<Folder> _subfolders;
	std::deque<Entry> _entries;
	Folder *_parent;
	// Disallow copying
	Folder (const Folder &) = delete;
	Folder &operator= (const Folder &) = delete;

	friend RootFolder_Ptr create_root_folder ();
};

/**
 * @brief Create a new root folder for a folder hierarchy
 * 
 * This method is necessary, because folders must NOT be allocated on the stack.
 */
inline RootFolder_Ptr create_root_folder () {
	return RootFolder_Ptr (new Folder());
}

struct SearchResult {
	
	inline SearchResult () : _match(0), _lastFolder(0), _lastIndex(-1) { }
	inline SearchResult (const Entry *e, const Folder *f, int ind) : _match(e), _lastFolder(f), _lastIndex(ind) { }
	
	inline const Entry *match () const {
		return _match;
	}
	inline bool hasMatch () const { return _match != 0; }
	
	inline const Folder *parentFolder () const { return _lastFolder; }
	
	inline int index () const { return _lastIndex; }
	
	const Entry *_match;
	const Folder *_lastFolder;
	int _lastIndex;
};

/**
 * @brief Search for entries in a folder hierarchy incrementally
 * @param searchString The string to search for. If it contains spaces, all words will be search independently
 * @param rootFolder Root-node of the hierarchy
 * @return Empty SearchResult-set if no matching entry was found
 */
SearchResult startSearch (const std::string &searchString, const Folder *rootFolder);
SearchResult continueSearch (const std::string &searchString, const SearchResult &lastResult);

/**
 * @brief Search folder by name in a folder hierarchy.
 * @param root The root object of the hierarchy to search
 * @param search_path Search path to follow, must begin with a slash. Example: "/test1/Folder 2"
 * @return 0 if the folder was not found.
 */
const Folder *get_folder_by_path (const Folder *root, const std::string &search_path);

/**
 * @brief Move a folder in the hierarchy
 * @param oldFolder Folder to be moved. Shall not be the root folder
 * @param newParent Parent folder to insert the folder to
 * @param newPosition the new element is inserted BEFORE this position in the list of the new parents subfolder. 
 */
Folder *moveFolder (Folder *oldFolder, Folder *newParent, int newPosition);

// Reader:

class Parser
{
public:
	bool 								readFile (std::istream &in, Folder *root);

	const std::string&					error () const;
private:
	void 								parse_folder (const Json::Value &folder, Folder *parent);
	void 								parse_folder_list (const Json::Value &item, Folder *parent);
	void 								parse_key (const Json::Value &key_entry, Folder *parent);
	
	std::string errorMsg;
};

// Writer

class Writer
{
public:
	bool 								writeFile (std::ostream &out, const Folder &root, bool write_formatted = false);

	const std::string&					error () const;
private:
	void 								serialize_folder (Json::Value &parent, const Folder &folder);
	
	std::string errorMsg;
};


// Impl

Entry::Entry (const std::string &title, const std::string &user, const std::string &url, const std::string &pwd, const std::string &comment) 
	: _title(title), _username (user), _url(url), _password (pwd), _comment (comment)
{}

}

#endif