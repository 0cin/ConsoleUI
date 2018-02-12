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
			InputController _ctrl;		// ��ȡ����װ��
			std::unique_ptr<ScrollBar>	// y��ƽ���ƶ�װ��
							_yScroller;
			std::unique_ptr<ScrollBar> 
							_xScroller;	// x��ƽ���ƶ�װ��
			Pen				_textColor;	// ������ɫ
			size_t			_length;	// ��ǰȫ�ǳ�
			Point			_cursor;	// ���λ��

		signals:
			sweet::Signal<void(TextBox*)> sig_TextChanged;
			sweet::Signal<void(TextBox*)> sig_TypeFinished;

		slots:
			sweet::Slot slot_CharRecv;
			sweet::Slot slot_FinishRecv;
			sweet::Slot slot_RectChanged;

		public:
			TextBox(
				Object*	parent = nullptr,								// ������
				Rect	rect = Rect(0, 0, 5, 5),						// �߽����
				short	borderWidth = 1,								// �߿���
				Bucket	border = Bucket(makeLpStream(L'*')),			// �߿��ַ���
				Bucket	vBorder = Bucket(makeLpStream(L'*')),			// ���б߿��ַ���
				Bucket	blank = Bucket(makeLpStream(L' ')),				// ����ַ���
				Pen		pen = Pen(makeLpStream(unsigned char(WHITE))),	// ����(��ɫ��)
				Pen		brush = Pen(makeLpStream(unsigned char(WHITE)))	// ��ˢ(��ɫ��)
			)
				: Frame(parent, rect, borderWidth, border,vBorder, blank, pen, brush)
				, _yScroller(nullptr)
				, _xScroller(nullptr)
				, _textColor(makeLpStream(cast2UChr(WHITE)))
				, _length(0)
			{
				moveCursor2Start();	// �ƶ��������ʼ��
				slot_CharRecv = _ctrl.sig_CharRecv.connect(this, &TextBox::treatNextChar);
				slot_FinishRecv = _ctrl.sig_RecvFinish.connect(this, &TextBox::receiveFinish);
				slot_RectChanged = sig_SetRect.connect(this, &TextBox::adaptRect);
			}
			
			/*void setText(const std::wstring& p);*/
			void appendChar(wchar_t ch) {
				short y{ getLineNumber() };
				if (!showNextChar(ch)) return;
				while (getLineNumber() >= _textTable.size()) {
					// �����һ���µ�
					_textTable.push_back(std::wstring());
					
				}
				// ��ĩ����ַ�
				_textTable[y].push_back(ch);
			}
			void startInput() { 
				tool::SetCursorVisible(true);	// ���ù��ɼ�
				_ctrl.go();		// ��ʼ������ť
			}
			void finishInput() { 
				tool::SetCursorVisible(false);	// ���ع��
				_ctrl.pause();	// ֹͣ������ť

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
			// ת������
			// ����InputController��׷���ź�
			void treatNextChar(InputController*, wchar_t ch) {
				appendChar(ch);	// ׷���ַ�
			}
			// ת������
			// ����InputController�Ľ����ź�
			void receiveFinish(InputController*) {
				finishInput();
			}
			// ����ַ���ȫ�ǳ���
			short getCharWidth(wchar_t chr) {
				return toShort(tool::IsFullWidthChar(chr)) + 1;
			}
			// ��ʾ�ַ�
			void showChar(wchar_t ch) {
				tool::ConsoleShow(std::wstring(1, ch), _cursor, WHITE);
				emit sig_TextChanged(this);
				// ���ù��λ��
				applyCursor();
			}
			void showChar(wchar_t ch, Point p) {
				tool::ConsoleShow(std::wstring(1, ch), _cursor, WHITE);
			}
			void showLine(const std::wstring& str, short y) {
			}
			// ��ʾ�ı�
			void showText() {
				short y = getPrintableY();
				for (auto x : _textTable)
					showLine(x, y++);
			}
			// �õ���ӡ��ʼ������
			// Ҳ���ǵ�һ�е�һ�е�����
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
			// ���¹��λ��
			void applyCursor() {
				tool::GotoXY(_cursor.x(), _cursor.y());
			}
			// �����ƶ�n�й��
			void cursorGoUp(short n) {
				_cursor.setY(_cursor.y() - n);
				applyCursor();
			}
			// �����ƶ�n�й��
			void cursorGoDown(short n) {
				cursorGoUp(-n);
			}
			// �����ƶ�n����λ���
			void cursorGoLeft(short n) {
				_cursor.setX(_cursor.x() - n);
				applyCursor();
			}
			// �����ƶ�n����λ���
			void cursorGoRight(short n) {
				cursorGoLeft(-n);
			}
			// ����ƶ�����������
			void moveCursor2Left() {
				_cursor.setX(getPrintableX());
				applyCursor();
			}
			// ����ƶ�����������
			void moveCursor2Right() {
				_cursor.setX(getPrintableX() + getDefaultPrinableLength());
				applyCursor();
			}
			// ����ƶ����������
			void moveCursor2Bottom() {
				_cursor.setY(getPrintableY() + getDefaultPrinableHeight());
				applyCursor();
			}
			// ����ƶ��������
			void moveCursor2Top() {
				_cursor.setY(getPrintableY());
				applyCursor();
			}
			// �ƶ���������
			void moveCursor2Start() {
				_cursor.setX(getPrintableX());
				_cursor.setY(getPrintableY());
				applyCursor();
			}
			// �õ���ǰ�к�
			short getLineNumber() {
				return _cursor.y() - getPrintableY();
			}
			// �õ���ǰ�к�
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
				// �õ��к��к�
				short y{ getLineNumber() };
				// �Ѿ�����
				if (y == 0 && _textTable[0].empty())
					return;
				if (_textTable[y].empty()) {	// �����ǰ��Ϊ��
					// ɾ������
					_textTable.erase(_textTable.begin() + y);
					cursorGoUp(1);	// �����ƶ�һ��
					moveCursor2Right();	// �ƶ������ұ�
					//cursorGoRight(1);
					// �����ƶ����ɸ� ��Ϊ���
					cursorGoLeft(getDefaultPrinableLength() - strLen(_textTable[y - 1]));
				}
				else {
					size_t size = _textTable[y].size();
					short lastCharLength = getCharWidth(_textTable[y][size - 1]);
					// ��������
					cursorGoLeft(getCharWidth(_textTable[y][size - 1]));
					// ɾ���ַ�
					_textTable[y].erase((++_textTable[y].rbegin()).base());
					
					// ����һ���ַ�
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
			// ׷����ʾ��һ���ַ�
			// ����true��ʾch���ַ������׷�ӣ�һ�㶼�ǿɴ�ӡ�ַ���
			bool showNextChar(wchar_t ch) {
				// �˸������
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
				Point	p{ getPrintableCrood() };	// �õ���ӡ����ʼ����
				// �õ��ɴ�ӡ��Χ�ڵ��кź��к�
				short	difX{ getLineNumber() }, difY{ getRowNumber() },
						charWidth{ getCharWidth(ch) };
				if (difX >= getDefaultPrinableHeight() && _yScroller.get() == nullptr)
					return false;
				// ������ϱ���Ҫ��ӡ���ַ���
				// ������һ�пɳ��ܵ���󳤶�
				if (difY + charWidth > getDefaultPrinableLength()) {
					// ���������һ��
					// �����ǰ�������������û�а�װy�����װ��
					if (difX + 1 == getDefaultPrinableHeight() && _yScroller.get() == nullptr) {
						// ������ӡ
						return false;
					}
					//_length = (difY + 1) * getDefaultPrinableLength();
					// �ƶ��ױ�����һ������
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
					// ��ʾ�ַ�
					showChar(ch);
					// �����ƶ�
					cursorGoRight(getCharWidth(ch));
				}
				return true;
			}
			

		};

	}
}