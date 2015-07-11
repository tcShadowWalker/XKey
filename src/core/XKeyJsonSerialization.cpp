#include "XKeyJsonSerialization.h"
#include "XKey.h"
#include <json/json.h>
#include <fstream>
#include <json/writer.h>
// Unix
#include <sys/stat.h>
#include <unistd.h>

namespace XKey {

// Parser:

struct ExceptionMaskReset
{
	ExceptionMaskReset (std::basic_ios<char> &ps) : s(ps), origState (s.exceptions()) {
		s.exceptions(origState | std::ios::badbit | std::ios::failbit);
	}
	~ExceptionMaskReset () {
		s.exceptions(origState);
	}
private:
	std::basic_ios<char> &s;
	std::ios::iostate origState;
};

bool Parser::read (std::istream &stream, Folder *new_folder_root) {
	if (!new_folder_root)
		throw std::invalid_argument("Need a root folder object to parse a file");
	
	if (!stream.good()) {
		this->errorMsg = "Could not open file";
		return false;
	}
	
	ExceptionMaskReset excMaskReset (stream);
	try {
		Json::Reader r;
		Json::Value json_root;
		if (r.parse(stream, json_root) == true) {
			parse_folder_list (json_root, new_folder_root);
			return true;
		} else {
			this->errorMsg = r.getFormattedErrorMessages();
			return false;
		}
	} catch (const std::exception &e) {
		this->errorMsg = e.what();
		stream.clear();
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
		email = key_entry.get("email", Json::Value::null),
		comment = key_entry.get("comment", Json::Value::null);
	if (!title.isString() || !user.isString() || !url.isString() || !pwd.isString() || !comment.isString())
		std::cerr << "Error when parsing key entry: Missing or invalid field\n";
	Entry ent (title.asString(), user.asString(), url.asString(), pwd.asString(),
		   (email.isString()) ? email.asString() : "",  comment.asString());
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

bool Writer::checkFilePermissions (const std::string &filename, bool *correctReadPermissions, bool *canWrite) {
	struct stat st;
	if (stat (filename.c_str(), &st) != 0) {
		if (errno == ENOENT)
			return false;
		throw std::runtime_error ("Failed to stat() file");
	}
	if (canWrite)
		*canWrite = (access (filename.c_str(), W_OK) == 0);
	// Not readable or writable by group or others
	*correctReadPermissions = ( (st.st_mode & (S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == 0);
	return true; // File exists
}

void Writer::setRestrictiveFilePermissions (const std::string &filename) {
	int r = chmod(filename.c_str(), S_IRUSR | S_IWUSR);
	if (r != 0)
		throw std::runtime_error ("Could not set permissions on file");
}

bool Writer::write (std::ostream &stream, const Folder &rootNode, bool write_formatted) {
	if (!stream.good()) {
		this->errorMsg = "Could not open file";
		return false;
	}
	ExceptionMaskReset excMaskReset (stream);
	
	Json::Value jsonRoot;
	serialize_folder (jsonRoot, rootNode);
	std::unique_ptr<Json::Writer> wptr;
	if (write_formatted)
		wptr.reset(new Json::StyledWriter);
	else
		wptr.reset(new Json::FastWriter);
	std::string text = wptr->write(jsonRoot);
	try {
		stream.write (text.data(), text.size());
	} catch (const std::exception &e) {
		this->errorMsg = e.what();
		return false;
	}
	if (!stream.good()) {
		errorMsg = "Could not write to output filestream";
		stream.clear();
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
