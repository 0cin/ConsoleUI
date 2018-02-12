#pragma once

#include <stack>
#include <cctype>
#include <vector>
#include <string>
#include <codecvt>
#include <Paint.h>
#include <Object.h>
#include <ScrollFrame.h>
#include <InputController.h>

namespace sweet {
	namespace cmd {

		class TextBox : public Frame {
		private:
			std::vector<std::wstring>
							_textTable;
			InputController _ctrl;		// 读取控制装置
			std::unique_ptr<ScrollBar>	// y轴平行移动装置
							_yScroller;
			std::unique_ptr<ScrollBar> 
							_xScroller;	// x轴平行移动装置
			Pen				_textColor;	// 文字颜色
			size_t			_length;	// 当前全角长
			Point			_cursor;	// 光标位置

		signals:
			sweet::Signal<void(TextBox*)> sig_TextChanged;
			sweet::Signal<void(TextBox*)> sig_TypeFinished;

		slots:
			sweet::Slot slot_CharRecv;
			sweet::Slot slot_FinishRecv;
			sweet::Slot slot_RectChanged;

		public:
			TextBox(
				Object*	parent = nullptr,								// 父部件
				Rect	rect = Rect(0, 0, 5, 5),						// 边界矩形
				short	borderWidth = 1,								// 边框宽度
				Bucket	border = Bucket(makeLpStream(L'*')),			// 边框字符流
				Bucket	vBorder = Bucket(makeLpStream(L'*')),			// 竖列边框字符流
				Bucket	blank = Bucket(makeLpStream(L' ')),				// 填充字符流
				Pen		pen = Pen(makeLpStream(unsigned char(WHITE))),	// 画笔(颜色流)
				Pen		brush = Pen(makeLpStream(unsigned char(WHITE)))	// 画刷(颜色流)
			)
				: Frame(parent, rect, borderWidth, border,vBorder, blank, pen, brush)
				, _yScroller(nullptr)
				, _xScroller(nullptr)
				, _textColor(makeLpStream(cast2UChr(WHITE)))
				, _length(0)
			{
				moveCursor2Start();	// 移动光标至开始处
				slot_CharRecv = _ctrl.sig_CharRecv.connect(this, &TextBox::treatNextChar);
				slot_FinishRecv = _ctrl.sig_RecvFinish.connect(this, &TextBox::receiveFinish);
				slot_RectChanged = sig_SetRect.connect(this, &TextBox::adaptRect);
			}
			
			/*void setText(const std::wstring& p);*/
			void appendChar(wchar_t ch) {
				short y{ getLineNumber() };
				if (!showNextChar(ch)) return;
				while (getLineNumber() >= _textTable.size()) {
					// 则添加一行新的
					_textTable.push_back(std::wstring());
					
				}
				// 行末添加字符
				_textTable[y].push_back(ch);
			}
			void startInput() { 
				tool::SetCursorVisible(true);	// 设置光标可见
				_ctrl.go();		// 开始监听按钮
			}
			void finishInput() { 
				tool::SetCursorVisible(false);	// 隐藏光标
				_ctrl.pause();	// 停止监听按钮

				emit sig_TypeFinished(this);
			}
			bool inputState() { return _ctrl.active(); }
			short getDefaultPrinableLength() { 
				return this->boundingRect().length() - 2 * getCharWidth(border().chrs()->ex());
			}
			short getDefaultPrinableHeight() { 
				return this->boundingRect().width() - 2 * borderWidth(); 
			}
			//size_t charCountLine(size_t numOfLine) const { return _text[numOfLine].size(); }

			virtual short identifier() { return 4; }
			virtual void paint(Object* obj) {
				Frame::paint(obj);
				short y = getPrintableY(), x = getPrintableX();
				for (auto str : _textTable) {
					//tool::GotoXY(x, y);
					if (y - getPrintableY() >= getDefaultPrinableHeight())
						break;
					tool::ConsoleShow(str, Point(x, y), WHITE);
					y++;
				}
			}

		protected:
			// 转发函数
			// 接受InputController的追加信号
			void treatNextChar(InputController*, wchar_t ch) {
				appendChar(ch);	// 追加字符
			}
			// 转发函数
			// 接受InputController的结束信号
			void receiveFinish(InputController*) {
				finishInput();
			}
			// 获得字符的全角长度
			short getCharWidth(wchar_t chr) {
				return toShort(tool::IsFullWidthChar(chr)) + 1;
			}
			// 显示字符
			void showChar(wchar_t ch) {
				tool::ConsoleShow(std::wstring(1, ch), _cursor, WHITE);
				emit sig_TextChanged(this);
				// 设置光标位置
				applyCursor();
			}
			void showChar(wchar_t ch, Point p) {
				tool::ConsoleShow(std::wstring(1, ch), _cursor, WHITE);
			}
			void showLine(const std::wstring& str, short y) {
			}
			// 显示文本
			void showText() {
				short y = getPrintableY();
				for (auto x : _textTable)
					showLine(x, y++);
			}
			// 得到打印开始的坐标
			// 也就是第一行第一列的坐标
			Point getPrintableCrood() {
				Point p = Map2Sense(this);
				//printf("%d %d\n", p.x(), p.y());
				p.setX(p.x() + borderWidth() * getCharWidth(border().chrs()->ex()));
				p.setY(p.y() + borderWidth());
				//printf("%d %d\n", p.x(), p.y());
				return p;
			}
			short getPrintableX() { return getPrintableCrood().x(); }
			short getPrintableY() { return getPrintableCrood().y(); }
			// 更新光标位置
			void applyCursor() {
				tool::GotoXY(_cursor.x(), _cursor.y());
			}
			// 向上移动n行光标
			void cursorGoUp(short n) {
				_cursor.setY(_cursor.y() - n);
				applyCursor();
			}
			// 向下移动n行光标
			void cursorGoDown(short n) {
				cursorGoUp(-n);
			}
			// 向左移动n个单位光标
			void cursorGoLeft(short n) {
				_cursor.setX(_cursor.x() - n);
				applyCursor();
			}
			// 向右移动n个单位光标
			void cursorGoRight(short n) {
				cursorGoLeft(-n);
			}
			// 光标移动至该行最左
			void moveCursor2Left() {
				_cursor.setX(getPrintableX());
				applyCursor();
			}
			// 光标移动至该行最右
			void moveCursor2Right() {
				_cursor.setX(getPrintableX() + getDefaultPrinableLength());
				applyCursor();
			}
			// 光标移动至该列最底
			void moveCursor2Bottom() {
				_cursor.setY(getPrintableY() + getDefaultPrinableHeight());
				applyCursor();
			}
			// 光标移动至该列最顶
			void moveCursor2Top() {
				_cursor.setY(getPrintableY());
				applyCursor();
			}
			// 移动光标至起点
			void moveCursor2Start() {
				_cursor.setX(getPrintableX());
				_cursor.setY(getPrintableY());
				applyCursor();
			}
			// 得到当前行号
			short getLineNumber() {
				return _cursor.y() - getPrintableY();
			}
			// 得到当前列号
			short getRowNumber() {
				return _cursor.x() - getPrintableX();
			}

			void adaptRect(Object* obj, Rect old, Rect newR) {
				if (_textTable.empty())
					return;
				short borderLen = borderWidth() * getCharWidth(border().chrs()->ex()) * 2;
				short len = newR.length() - borderLen;

				std::vector<std::wstring> _nTextTable;
				std::wstring tmp;
				for (auto str : _textTable) {
					for (auto chr : str) {
						if (tmp.length() < len) {
							tmp += chr;
						}
						else {
							_nTextTable.push_back(tmp);
							tmp.clear();
						}
					}
				}
				if (!tmp.empty())
					_nTextTable.push_back(tmp);
				_textTable = _nTextTable;

				refresh();
			}
			

			void enter() {
				short y{ getLineNumber() };
				if (y + 1 < getDefaultPrinableHeight()) {
					cursorGoDown(1);
					moveCursor2Left();
				}
			}
			void backSpace() {
				if (_textTable.empty())
					return;
				// 得到行号列号
				short y{ getLineNumber() };
				// 已经空了
				if (y == 0 && _textTable[0].empty())
					return;
				if (_textTable[y].empty()) {	// 如果裆前行为空
					// 删除此行
					_textTable.erase(_textTable.begin() + y);
					cursorGoUp(1);	// 向上移动一行
					moveCursor2Right();	// 移动至最右边
					//cursorGoRight(1);
					// 向左移动若干格 因为有填补
					cursorGoLeft(getDefaultPrinableLength() - strLen(_textTable[y - 1]));
				}
				else {
					size_t size = _textTable[y].size();
					short lastCharLength = getCharWidth(_textTable[y][size - 1]);
					// 正常左移
					cursorGoLeft(getCharWidth(_textTable[y][size - 1]));
					// 删除字符
					_textTable[y].erase((++_textTable[y].rbegin()).base());
					
					// 后退一个字符
					putchar(' ');
					for (short i = 0; i <= lastCharLength; ++i)
						putchar('\b');
				}
			}
			void tab() {

			}
			void left() {

			}
			void right() {

			}
			void up() {

			}
			void down() {

			}
			// 追加显示下一个字符
			// 返回true表示ch的字符宽可以追加（一般都是可打印字符）
			bool showNextChar(wchar_t ch) {
				// 退格键处理
				switch (ch) {
				case '\b':
					backSpace();
					return false;

				case '\r':
					enter();
					return false;

				case '\t':
					tab();
					return false;
					
				case VK_UP:
					up();
					return false;

				case VK_DOWN:
					down();
					return false;

				case VK_LEFT:
					left();
					return false;

				case VK_RIGHT:
					right();
					return false;
				}
				Point	p{ getPrintableCrood() };	// 得到打印的起始坐标
				// 得到可打印范围内的行号和列号
				short	difX{ getLineNumber() }, difY{ getRowNumber() },
						charWidth{ getCharWidth(ch) };
				if (difX >= getDefaultPrinableHeight() && _yScroller.get() == nullptr)
					return false;
				// 如果加上本次要打印的字符宽
				// 大于这一行可承受的最大长度
				if (difY + charWidth > getDefaultPrinableLength()) {
					// 换行输出下一行
					// 如果当前列已是最大列且没有安装y轴滚动装置
					if (difX + 1 == getDefaultPrinableHeight() && _yScroller.get() == nullptr) {
						// 放弃打印
						return false;
					}
					//_length = (difY + 1) * getDefaultPrinableLength();
					// 移动胱标至下一行最左
					cursorGoDown(1);
					moveCursor2Left();
					showChar(ch);
				}
				else if (difY + charWidth == getDefaultPrinableLength()) {
					showChar(ch);
					cursorGoDown(1);
					moveCursor2Left();
				}
				else {
					// 显示字符
					showChar(ch);
					// 向右移动
					cursorGoRight(getCharWidth(ch));
				}
				return true;
			}
			

		};

	}
}