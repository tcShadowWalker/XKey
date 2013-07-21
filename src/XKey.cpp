#include "XKey.h"

#include <json/json.h>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <json/writer.h>

namespace XKey {

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

/*void Folder::addSubfolder (Folder folder) {
	_subfolders.insert (_subfolders.end(), std::move(folder));
}*/

Folder *Folder::createSubfolder (const std::string &name) {
	_subfolders.emplace_back (name, this);
	return &_subfolders.back();
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
	Json::Value user = key_entry.get("username", Json::Value::null),
		url = key_entry.get("url", Json::Value::null), pwd = key_entry.get("password", Json::Value::null);
	if (!user.isString() || !url.isString() || !pwd.isString())
		throw std::runtime_error ("Could not parse key entry: Missing field");
	Entry ent (user.asString(), url.asString(), pwd.asString());
	parent->addEntry (std::move(ent));
}

void Parser::parse_folder (const Json::Value &folder, Folder *parent) {
	Json::Value name = folder.get("name", Json::Value::null);
	if (!name.isString())
		throw std::runtime_error ("Invalid subfolder entry: missing name");
	// Create info
	Folder *f = parent->createSubfolder(name.asString());
	//Folder f (name.asString());
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
			key["username"] = it.username();
			key["url"] = it.url();
			key["password"] = it.password();
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
