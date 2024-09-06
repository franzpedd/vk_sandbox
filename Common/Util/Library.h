#pragma once

#include <unordered_map>

namespace Cosmos
{
	template<typename T>
	class Library
	{
	public:

		// constructor
		Library() = default;

		// destructor
		~Library()
		{
			mContent.clear();
		}

		// returns all objects ref
		inline std::unordered_map<const char*, T>& GetAllRefs() { return mContent; }

	public:

		// checks if a given content/element exists within the library
		bool Exists(const char* name)
		{
			return mContent.find(name) != mContent.end() ? true : false;
		}

		// inserts a new content/element into the library
		void Insert(const char* name, T object)
		{
			if (mContent.find(name) == mContent.end()) {
				mContent[name] = object;
			}
		}

		// deletes a content/element from the library
		void Remove(const char* name)
		{
			if (Exists(name)) {
				mContent.erase(name);
			}
		}

		// erases a given content/element from the library
		void Erase(const char* name)
		{
			if (mContent.find(name) != mContent.end()) {
				mContent.erase(name);
			}
		}

		// returns the object reference
		T& GetRef(const char* name)
		{
			if (!Exists(name)) { // cannot return a reference for a non-existent content
				std::abort();
			}

			return mContent[name];
		}

	private:

		std::unordered_map<const char*, T> mContent;
	};
}