#pragma once

#include <string>
#include <iostream>

namespace Json { class Value; }

namespace XKey {

class Folder;

/**
 * @brief Reader to parse XKey structures from cleartext streams
 */
class Parser
{
public:
	/**
	 * 
	 */
	bool read (std::istream &in, Folder *root);

	const std::string& error () const;
private:
	void parse_folder (const Json::Value &folder, Folder *parent);
	void parse_folder_list (const Json::Value &item, Folder *parent);
	void parse_key (const Json::Value &key_entry, Folder *parent);
	
	std::string errorMsg;
};

/**
 * @brief Writer to write XKey structures to cleartext streams
 */
class Writer
{
public:
	enum WriterFlags {
		WRITE_NONE = 0,
		/// Write formatted Json output
		WRITE_FORMATTED = 1,
	};

	bool write (std::ostream &out, const Folder &root, int flags = WRITE_NONE);

	const std::string& error () const;

	/**
	 * @brief Assert correct file permissions
	 * @param filename full pathname
	 * @param correctReadPermissions if not NULL, check if read permissions are set correctly (only owner can read)
	 * @param canWrite if not NULL, containts whether the application can write to the file
	 * @return true iff file exists
	 */
	static bool checkFilePermissions (const std::string &filename, bool *correctReadPermissions, bool *canWrite = nullptr);

	/// Make a file readable and writable only by it's owner
	static void setRestrictiveFilePermissions (const std::string &filename);
	
	/**
	 * @brief Move a file in the file-system, preserving it's attributes
	 */
	static void moveFile (const std::string &sourceFile, const std::string &targetFile);
	
	/**
	 * @brief Remove a file from the filesystem
	 */
	static void removeFile (const std::string &file);
private:
	void serialize_folder (Json::Value &parent, const Folder &folder);
	
	std::string errorMsg;
};

}
