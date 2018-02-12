#pragma once

#include <vector>
#include <thread>
#include <memory>
#include <functional>
#include <Windows.h>
#include <Controller.h>

namespace sweet {
	namespace cmd {

		class TimeController : public Controller {
			typedef std::function<void()> callBackType;
		private:
			size_t							_breath;
			std::unique_ptr<std::thread>	_thr;
			std::vector<callBackType>		_callbacks;

		public:
			TimeController(size_t delayTime)
				: _breath(delayTime)
				, _thr(nullptr) {}

			template < class... Args >
			void add(void(*func)(Args...), Args&&... args) {
				_callbacks.push_back(std::bind(func, std::forward<Args>(args)...));
			}
			void setBreathTime(size_t t) { _breath = t; }

			virtual void go();
			virtual void pause() { _thr.reset(nullptr); }
			virtual bool active() { return nullptr != _thr.get(); }
		};

		void TimeController::go() {
			if (nullptr == _thr.get())
				_thr.reset(new std::thread);

			_thr.get()->swap(std::thread([&]() {
				for (;;) {
					for (auto wptr : this->_callbacks) {
						wptr();
					}
					Sleep(this->_breath);
				}
			}));
			_thr->detach();
		}

	}
}