#pragma once

#include <string>
#include <deque>
#include <memory>

namespace XKey {

class Folder;

/**
 * @brief A single key entry
 */
class Entry
{
public:
	inline Entry () { }
	inline Entry (const std::string &title, const std::string &user, const std::string &url,
		      const std::string &pwd, const std::string &email, const std::string &comment);
	
	const std::string& title() const { return _title; }
	const std::string& username() const { return _username; }
	const std::string& url() const { return _url; }
	const std::string& password() const { return _password; }
	const std::string& email() const { return _email; }
	const std::string& comment() const { return _comment; }
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
	const std::string& name() const { return _name; }
	Folder* parent() const { return _parent; }
	
	void setName (const std::string &name);
	
	std::string fullPath() const;
	
	void addEntry (Entry entry);
	Entry& getEntryAt (int index);
	void removeEntry (int index);
	Folder* createSubfolder (const std::string &name);
	void removeSubfolder (int index);
	
	Folder* getSubfolder (const std::string &name);
	const Folder* getSubfolder (const std::string &name) const;
	
	const std::deque<Folder>& subfolders () const { return _subfolders; }
	const std::deque<Entry>& entries () const { return _entries; }
	
	std::deque<Folder>& subfolders () { return _subfolders; }
	std::deque<Entry>& entries () { return _entries; }
	
	/// @return this items index in the parent's folder-list
	int row() const;
	
	void operator= (Folder &&);
private:
	std::string _name;
	std::deque<Folder> _subfolders;
	std::deque<Entry> _entries;
	Folder *_parent;
	
	Folder ();
	// Disallow copying
	Folder (const Folder &) = delete;
	Folder &operator= (const Folder &) = delete;

	friend RootFolder_Ptr createRootFolder ();
	struct construct_key {};
public:
	/// Only to be called internally
	Folder (const std::string &name, Folder *parent, const construct_key &);
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
 * An object of this class gets returned by #continueSearch or #startSearch.\n
 * It serves as an iterator into the key-tree and can be given as a parameter
 * to #continueSearch to return the next result.\n
 * \n
 * You must not modify the tree-structure while performing a search, or the result is undefined.
 */
struct SearchResult
{
	SearchResult () : _match(0), _lastFolder(0), _lastIndex(-1) { }
	SearchResult (const Entry *e, const Folder *f, int ind) : _match(e), _lastFolder(f), _lastIndex(ind) { }
	
	const Entry *match () const {
		return _match;
	}
	
	/// Check if the SearchResult contains a valid result match
	bool hasMatch () const { return _match != 0; }
	
	const Folder *parentFolder () const { return _lastFolder; }
	
	int index () const { return _lastIndex; }
	
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


// Impl

Entry::Entry (const std::string &title, const std::string &user, const std::string &url,
	      const std::string &pwd, const std::string &email, const std::string &comment)
	: _title(title), _username (user), _url(url), _password (pwd), _email(email), _comment (comment)
{}

}
