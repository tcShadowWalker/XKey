#include "XKey.h"

#include <json/json.h>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <cassert>
#include <json/writer.h>

namespace XKey {

static void tokenize (const std::string &str, std::vector<std::string> *tokens, const char delimiters) {
	std::string::size_type lastPos = str.find_first_not_of (delimiters, 0);
	std::string::size_type pos = str.find_first_of (delimiters, lastPos);
	while (pos != std::string::npos || lastPos != std::string::npos) {
		tokens->push_back (str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}
}

const XKey::Folder *search_folder (const XKey::Folder *root, const std::string &search_path) {
	std::vector<std::string> searchPathComponents;
	std::vector<std::string>::const_iterator currentSearchPathComponent;
	if (search_path.size() > 0) {
		tokenize(search_path, &searchPathComponents, '/');
		currentSearchPathComponent = searchPathComponents.begin();
	}
	const XKey::Folder *current = root;
	while (currentSearchPathComponent != searchPathComponents.end() && current) {
		current = current->getSubfolder(*currentSearchPathComponent++);
	}
	return current;
}

// Folder:

Folder::Folder () :_parent(0) { }

Folder::Folder (const std::string &name, Folder *par) : _name(name), _parent(par) {}

std::string	Folder::fullPath () const {
	std::string fp;
	const Folder *p = this;
	// If p->_parent == 0, we have the nameless root of the tree
	while (p->_parent != 0) {
		fp.insert (0, "/");
		fp.insert (1, p->name());
		p = p->_parent;
	}
	return fp;
}

void Folder::addEntry (Entry entry) {
	_entries.insert (_entries.end(), std::move(entry));
}

void Folder::removeEntry (int index) {
	if (index < 0 || index >= _entries.size())
		throw std::logic_error ("Invalid key-entry index: Can not be removed");
	_entries.erase(_entries.begin() + index);
}

Folder *Folder::createSubfolder (const std::string &name) {
	if (getSubfolder(name))
		throw std::logic_error ("Can not create a second folder with the same name within the same parent");
	_subfolders.emplace_back (name, this);
	return &_subfolders.back();
}

Folder *Folder::getSubfolder (const std::string &name) {
	return const_cast<Folder*> ( ((const Folder*)this)->getSubfolder(name) );
}

const Folder *Folder::getSubfolder (const std::string &name) const {
	std::deque<Folder>::const_iterator it = std::find_if(_subfolders.begin(), _subfolders.end(), [name] (const XKey::Folder &f) {
												return (f.name() == name);
											} );
	return (it == _subfolders.end()) ? 0 : &*it;
}

int Folder::row() const {
	if (_parent) {
		std::deque<Folder>::const_iterator it = std::find_if(_parent->_subfolders.begin(), _parent->_subfolders.end(), [this] (const XKey::Folder &f) {
												return (&f == this);
											} );
		assert (it != _parent->_subfolders.end());
		return it - _parent->_subfolders.begin();
	}
	return 0;
}

void Folder::setName (const std::string &name) {
	this->_name = name;
}

// Parser:

bool Parser::readFile (std::istream &stream, Folder *new_folder_root) {
	if (!new_folder_root)
		throw std::logic_error("Need a root folder object to parse a file");
	
	if (!stream.good()) {
		this->errorMsg = "Could not open file";
		return false;
	}

	Json::Reader r;
	Json::Value json_root;
	if (r.parse(stream, json_root) == true) {
		parse_folder_list (json_root, new_folder_root);
		return true;
	} else {
		this->errorMsg = r.getFormattedErrorMessages();
		return false;
	}
}

const std::string &Parser::error () const {
	return errorMsg;
}

void Parser::parse_key (const Json::Value &key_entry, Folder *parent) {
	Json::Value title = key_entry.get("title", Json::Value::null),
		user = key_entry.get("username", Json::Value::null),
		url = key_entry.get("url", Json::Value::null),
		pwd = key_entry.get("password", Json::Value::null),
		comment = key_entry.get("comment", Json::Value::null);
	if (!title.isString() || !user.isString() || !url.isString() || !pwd.isString() || !comment.isString())
		throw std::runtime_error ("Could not parse key entry: Missing field");
	Entry ent (title.asString(), user.asString(), url.asString(), pwd.asString(), comment.asString());
	parent->addEntry (std::move(ent));
}

void Parser::parse_folder (const Json::Value &folder, Folder *parent) {
	Json::Value name = folder.get("name", Json::Value::null);
	if (!name.isString())
		throw std::runtime_error ("Invalid subfolder entry: missing name");
	// Create info
	Folder *f = parent->createSubfolder(name.asString());
	Json::Value keys = folder.get("keys", Json::Value::null);
	if (keys.isArray()) {
		for (const auto &it : keys) {
			if (!it.isObject())
				throw std::runtime_error ("Invalid key-entry in folder");
			parse_key (it, f);
		}
	}
	// Parse subfolders:
	parse_folder_list (folder, f);
}

void Parser::parse_folder_list (const Json::Value &item, Folder *parent) {
	Json::Value list = item.get("folders", Json::Value::null);
	if (list.isArray()) {
		for (const auto &it : list) {
			if (!it.isObject())
				throw std::runtime_error ("Invalid entry in subfolder list");
			parse_folder (it, parent);
		}
	}
}

// Writer

bool Writer::writeFile (std::ostream &stream, const Folder &rootNode, bool write_formatted) {
	if (!stream.good()) {
		this->errorMsg = "Could not open file";
		return false;
	}
	
	Json::Value jsonRoot;
	serialize_folder (jsonRoot, rootNode);
	std::unique_ptr<Json::Writer> wptr;
	if (write_formatted)
		wptr.reset(new Json::StyledWriter);
	else
		wptr.reset(new Json::FastWriter);
	std::string text = wptr->write(jsonRoot);
	stream.write (text.data(), text.size());
	if (!stream.good()) {
		errorMsg = "Could not write to output filestream";
		return false;
	}
	return true;
}

const std::string& Writer::error () const {
	return errorMsg;
}
void Writer::serialize_folder (Json::Value &parent, const Folder &folder) {
	parent["name"] = Json::Value(folder.name());
	if (!folder.entries().empty()) {
		Json::Value keys (Json::arrayValue);
		for (const auto &it : folder.entries()) {
			Json::Value key;
			key["title"] = it.title();
			key["username"] = it.username();
			key["url"] = it.url();
			key["password"] = it.password();
			key["comment"] = it.comment();
			keys.append(key);
		}
		parent["keys"] = keys;
	}
	if (!folder.subfolders().empty()) {
		Json::Value subfolders (Json::arrayValue);
		for (const auto &it : folder.subfolders()) {
			Json::Value v (Json::objectValue);
			serialize_folder(v, it);
			subfolders.append(v);
		}
		parent["folders"] = subfolders;
	}
}



}
