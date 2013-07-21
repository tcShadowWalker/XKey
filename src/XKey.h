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
	inline Entry (const std::string &user, const std::string &url, const std::string &pwd);
	
	inline const std::string&			username() const { return _username; }
	inline const std::string&			url() const { return _url; }
	inline const std::string&			password() const { return _password; }
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
	
	std::string							fullPath() const;
	
	void 								addEntry (Entry entry);
	Folder *							createSubfolder (const std::string &name);
	//void 								addSubfolder (Folder folder);
	
	inline const std::deque<Folder>&	subfolders () const { return _subfolders; }
	inline const std::deque<Entry>&		entries () const { return _entries; }
private:
	std::string _name;
	std::deque<Folder> _subfolders;
	std::deque<Entry> _entries;
	Folder *_parent;
};

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

Entry::Entry (const std::string &user, const std::string &url, const std::string &pwd) 
	: _username (user), _url(url), _password (pwd)
{}

}

#endif