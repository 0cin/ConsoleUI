#pragma once

#include <map>
#include <string>

namespace sweet {

	namespace cmd {

		class ConsoleHelper
		{
			HANDLE _hIn;
			HANDLE _hOut;
			INPUT_RECORD _InRec;
			DWORD _NumRead;
		public:
			WORD VKey;
			ConsoleHelper(void) {
				_hIn = GetStdHandle(STD_INPUT_HANDLE);
				_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
				VKey = 0;
			}
			bool ReadOneInput()
			{
				return 0 != ReadConsoleInput(_hIn, &_InRec, 1, &_NumRead);
			}
			bool ReadOneInput(INPUT_RECORD& InRec)
			{
				return 0 != ReadConsoleInput(_hIn, &InRec, 1, &_NumRead);
			}
			DWORD ReadKeyDown()
			{
				if (!ReadConsoleInput(_hIn, &_InRec, 1, &_NumRead))
					return 0;
				if (_InRec.EventType != KEY_EVENT)
					return 0;
				if (_InRec.Event.KeyEvent.bKeyDown > 0)
					return 0;
				VKey = _InRec.Event.KeyEvent.wVirtualKeyCode;
				return VKey;
			}
			DWORD ReadKeyPush()
			{
				if (!ReadConsoleInput(_hIn, &_InRec, 1, &_NumRead))
					return 0;
				if (_InRec.EventType != KEY_EVENT)
					return 0;
				if (_InRec.Event.KeyEvent.bKeyDown == 0)
					return 0;
				VKey = _InRec.Event.KeyEvent.wVirtualKeyCode;
				return VKey;
			}
		public:
			~ConsoleHelper(void) {}
		};

		class Controller {
		private:
			std::map<std::string, Controller*> _friends;

		public:
			virtual ~Controller() = default;

			Controller*  getFriendWithTag(const std::string& tag) {
				if (!_friends.count(tag))
					return nullptr;
				return _friends[tag];
			}
			bool		 hasTag(const std::string& tag) {
				return 0 < _friends.count(tag);
			}
			bool		 removeFriend(const std::string& tag) {
				if (hasTag(tag)) {
					_friends.erase(_friends.find(tag));
					return true;
				}
				return false;
			}
			bool		 addFriend(Controller* ctrl, const std::string& tag) {
				if (hasTag(tag))
					return false;
				_friends.insert(std::make_pair(tag, ctrl));
				return true;
			}
			std::string  find(Controller* ctrl) {
				for (auto itr = _friends.begin(); _friends.end() != itr; ++itr) {
					if (itr->second == ctrl)
						return itr->first;
				}
				return std::string();
			}

			virtual void go() = 0;
			virtual void pause() = 0;
			virtual bool active() = 0;
		};
	}

}