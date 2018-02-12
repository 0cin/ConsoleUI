#pragma once

#include <thread>
#include <memory>
#include <codecvt>
#include <conio.h>
#include <Object.h>
#include <Signal.h>
#include <Controller.h>

#define GBK_NAME ".936" 

namespace sweet {
	namespace cmd {
		class InputController : public Controller {
		private:
			int				_keyEscape;
			std::unique_ptr<std::thread> _thr;
			bool			_state;

		signals:
			sweet::Signal<void(InputController*, wchar_t)> sig_CharRecv;
			sweet::Signal<void(InputController*)> sig_RecvFinish;

		public:
			InputController(int key2Escape = VK_ESCAPE)
				: _keyEscape(key2Escape) {}

			virtual void go() {
				if (nullptr == _thr.get())
					_thr.reset(new std::thread());
				_state = true;
				_thr->swap(std::thread([&]() {
					char ch;
					bool chinese{ false };
					std::string chineseBuffer;
					if (_state) {
						while (ch = _getch()) {
							if (ch == _keyEscape)
								break;
							// 控制字符前缀
							if (ch == -32)
								continue;
							if (ch < 0) {	// 接到一个负数
								if (!chinese) {
									chineseBuffer += ch;
									chinese = true;
									continue;
								}
								if (chinese) {
									chineseBuffer += ch;
									chinese = false;
									using wcharGBK = std::codecvt_byname<wchar_t, char, mbstate_t>;
									std::wstring_convert<wcharGBK>
										_cvtGBK(new wcharGBK(GBK_NAME));	// 异世界の编码装换装置

									std::wstring tmp(_cvtGBK.from_bytes(chineseBuffer));
									emit sig_CharRecv(this, tmp[0]);
									chineseBuffer.clear();
								}
							}
							else emit sig_CharRecv(this, ch);
						}
					}
					emit sig_RecvFinish(this);
				}));
				_thr->detach();
			}
			virtual void pause() {
				_state = false;
			}
			virtual bool active() {
				return _state;
			}

		};
	}
}