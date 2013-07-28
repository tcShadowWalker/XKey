#ifndef XKEY_H
#define XKEY_H

#include <string>
#include <deque>

namespace Json {
	class Value;
}

namespace XKey {

class Entry
{
public:
	// TODO title, comment!
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

class Folder
{
public:
	Folder ();
	Folder (const std::string &name, Folder *parent);

	inline const std::string&			name() const { return _name; }
	inline Folder*						parent() const { return _parent; }
	
	void 								setName (const std::string &name);
	
	std::string							fullPath() const;
	
	
	/// Recursively counts subfolders
	int 								countSubfolders () const;
	
	void 								addEntry (Entry entry);
	Folder *							createSubfolder (const std::string &name);
	
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
};

/**
 * @brief Search folder by name in a folder hierarchy.
 * @param root The root object of the hierarchy to search
 * @param search_path Search path to follow, must begin with a slash. Example: "/test1/Folder 2"
 * @return 0 if the folder was not found.
 */
const XKey::Folder *search_folder (const XKey::Folder *root, const std::string &search_path);

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