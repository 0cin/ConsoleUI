#pragma once

#include <thread>
#include <memory>
#include <Signal.h>
#include <Windows.h>
#include <Controller.h>

namespace sweet {
	namespace cmd {
		class KeyController : public Controller {
		private:
			std::unique_ptr<std::thread> _thr;	// 线程(执行按键判断)
			ConsoleHelper				 _gch;	// 辅助对象

		public:
			virtual ~KeyController() = default;

			virtual void go();
			virtual void pause() { _thr.reset(nullptr); }
			virtual bool active() { return nullptr != _thr.get(); }

		signals:
			sweet::Signal<void(Controller*, int)> sig_KeyPressed;

		};

		void KeyController::go() {
			if (nullptr == _thr.get())				// 如果线程没有创建
				_thr.reset(new std::thread);

			_thr.get()->swap(std::thread([&]() {	// 创建 + 启动线程
				for (; ;) {
					// 检测按键 推送信号
					if (_gch.ReadKeyPush() != 0) {
						// 推送按键消息
						sig_KeyPressed(this, _gch.VKey);
					}
					Sleep(10);	// take a breath.
				}
			}));
			_thr.get()->detach();
		}

		class KeyLastController : public Controller {
		private:
			bool			_codesMap[256];
			std::unique_ptr<std::thread>
				_thr;
			ConsoleHelper	_gch;
			bool			_state;

		public:
			KeyLastController() {}
			virtual ~KeyLastController() = default;

			virtual void go();
			virtual void pause() { _state = false; }
			virtual bool active() { return _state; }

		signals:
			sweet::Signal<void(KeyLastController*, int, bool)> sig_KeyPressed;
		};

		void KeyLastController::go() {
			if (nullptr == _thr.get())				// 如果线程没有创建
				_thr.reset(new std::thread);

			_state = true;
			_thr->swap(std::thread([&]() {
				for (; _state;) {
					// 按键按下
					if (!_codesMap[_gch.VKey] && 0 != _gch.ReadKeyPush()) {
						_codesMap[_gch.VKey] = true;
						sig_KeyPressed(this, _gch.VKey, true);
					}
					// 按键弹起
					if (_codesMap[_gch.VKey] && 0 != _gch.ReadKeyDown()) {
						_codesMap[_gch.VKey] = false;
						sig_KeyPressed(this, _gch.VKey, false);
					}
					Sleep(10);
				}
			}));
			_thr->detach();
		}
		
	}
}