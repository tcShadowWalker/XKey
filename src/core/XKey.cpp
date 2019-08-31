#include "XKey.h"

#include <algorithm>
#include <stdexcept>
#include <memory>
#include <cassert>
#include <vector>

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

static bool compareCaseInsensitive (char a, char b) {
	return std::tolower(a) == std::tolower (b);
}

static inline bool match_in_string (const std::string &needle, const std::string &haystack) {
	return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), &compareCaseInsensitive) != haystack.end();
}

static SearchResult search_folder (const std::vector<std::string> &tokens, const Folder *f, int begin_index = 0) {
	if (begin_index > f->entries().size())
		throw std::invalid_argument ("begin_index parameter for search_folder greater than number of sumfolders");
	int ind = begin_index;
	for (auto ent = f->entries().begin() + begin_index; ent != f->entries().end(); ++ent) {
		for (const std::string &word : tokens) {
			if (match_in_string (word, ent->title()) || match_in_string(word, ent->comment()) ||
			    match_in_string(word, ent->url()) || match_in_string(word, ent->username()) ||
			    match_in_string(word, ent->email()))
			{
				return SearchResult (&*ent, f, ind);
			}
		}
		++ind;
	}
	return SearchResult();
}

static SearchResult search_down_recursive (const std::vector<std::string> &tokens, const Folder *startFolder) {
	// First look through all entries in THIS folder
	SearchResult e = search_folder (tokens, startFolder);
	if (e.hasMatch())
		return e;
	// Now look through all subfolders
	for (const Folder &s : startFolder->subfolders()) {
		SearchResult e = search_down_recursive(tokens, &s);
		if (e.hasMatch())
			return e;
	}
	return SearchResult();
}

static SearchResult search_up_recursive (const std::vector<std::string> &tokens, const Folder *lastFolder) {
	const XKey::Folder *p = lastFolder->parent();
	// Start with the first SIBLING of lastFolder
	for (std::deque<Folder>::const_iterator folderIt = p->subfolders().begin() + lastFolder->row()+1;
	     folderIt < p->subfolders().end(); ++folderIt)
	{
		SearchResult e = search_down_recursive(tokens, &*folderIt);
		if (e.hasMatch())
			return e;
	}
	// Go up the hierarchy
	if (p->parent())
		return search_up_recursive (tokens, p);
	else
		return SearchResult();
}

SearchResult continueSearch (const std::string &searchString, const SearchResult &lastResult) {
	if (!lastResult._lastFolder)
		return SearchResult();
	std::vector<std::string> tokens;
	tokenize (searchString, &tokens, ' ');
	const XKey::Folder *startFolder = lastResult._lastFolder;
	// Look for siblings from the first result
	SearchResult res = search_folder (tokens, startFolder, lastResult._lastIndex + 1);
	if (res.hasMatch())
		return res;
	// Now look ABOVE this folder.
	if (startFolder->parent())
		return search_up_recursive (tokens, startFolder);
	return SearchResult { }; // No match
}

SearchResult startSearch (const std::string &searchString, const XKey::Folder *startFolder) {
	std::vector<std::string> tokens;
	tokenize (searchString, &tokens, ' ');
	return search_down_recursive(tokens, startFolder);
}

const XKey::Folder *getFolderByPath (const XKey::Folder *root, const std::string &search_path) {
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

Folder::Folder (const std::string &name, Folder *par, const construct_key &) : _name(name), _parent(par) {}

void Folder::operator= (Folder &&o) {
	_name = std::move(o._name);
	_entries = std::move(o._entries);
	_subfolders = std::move(o._subfolders);
	// Fix subfolders parent-ptr
	for (auto &it : _subfolders)
		it._parent = this;
}

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
		throw std::invalid_argument ("Invalid key-entry index: Can not be removed");
	_entries.erase(_entries.begin() + index);
}

Entry &Folder::getEntryAt (int index) {
	if (index < 0 || index >= _entries.size())
		throw std::invalid_argument ("Invalid key-entry index: Can not be found");
	return *(_entries.begin() + index);
}

Folder *Folder::createSubfolder (const std::string &name) {
	if (getSubfolder(name))
		throw std::invalid_argument ("Can not create a second folder with the same name within the same parent");
	_subfolders.emplace_back (name, this, construct_key{});
	return &_subfolders.back();
}

void Folder::removeSubfolder (int index) {
	if (index < 0 || index >= subfolders().size())
		throw std::invalid_argument ("Invalid subfolder index: Can not be removed");
	_subfolders.erase(_subfolders.begin() + index);
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
		std::deque<Folder>::const_iterator it = std::find_if(_parent->_subfolders.begin(), _parent->_subfolders.end(),
			[this] (const XKey::Folder &f) { return (&f == this); } );
		assert (it != _parent->_subfolders.end());
		return it - _parent->_subfolders.begin();
	}
	return 0;
}

void Folder::setName (const std::string &name) {
	this->_name = name;
}

Folder *moveFolder (Folder *oldFolder, Folder *newParent, int newPosition) {
	Folder *oldParent = oldFolder->parent();
	if (!oldParent)
		return oldFolder;
	XKey::Folder *newFolder = newParent->createSubfolder(oldFolder->name());
	*newFolder = std::move(*oldFolder);
	oldParent->removeSubfolder(oldFolder->row());
	return newFolder;
}

}
