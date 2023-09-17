// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include <Application.h>
#include <Clipboard.h>
#include <ColumnListView.h>
#include <ColumnTypes.h>
#include <LayoutBuilder.h>


class KeyListView : public BColumnListView {
public:
	KeyListView()
		:
		BColumnListView("Key Output", 0, B_FANCY_BORDER, true)
	{
		AddColumn(new (std::nothrow) BStringColumn("Event", 50, 40, 100, B_TRUNCATE_MIDDLE), 0);
		AddColumn(new (std::nothrow) BIntegerColumn("Key", 40, 40, 100), 1);
		AddColumn(new (std::nothrow) BStringColumn("Modifiers", 150, 40, 220, B_TRUNCATE_MIDDLE), 2);
		AddColumn(new (std::nothrow) BIntegerColumn("States", 50, 40, 100), 3);
		AddColumn(new (std::nothrow) BIntegerColumn("Byte", 60, 40, 100), 4);
		AddColumn(new (std::nothrow) BStringColumn("Bytes", 80, 40, 200, B_TRUNCATE_MIDDLE), 5);
		AddColumn(new (std::nothrow) BIntegerColumn("Raw", 70, 40, 200), 6);
		AddColumn(new (std::nothrow) BIntegerColumn("Repeat", 70, 40, 200), 7);
		SetSortingEnabled(false);
		SetSelectionMode(B_SINGLE_SELECTION_LIST);
	}


	virtual void AttachedToWindow()
	{
		MakeFocus(true);
		BColumnListView::AttachedToWindow();
	}


	virtual void SelectionChanged()
	{
		BRow* selected = CurrentSelection();
		if (selected == NULL)
			return;

		BIntegerField* field = dynamic_cast<BIntegerField*>(selected->GetField(1));
		if (field == NULL)
			return;

		if (!be_clipboard->Lock())
			return;

		be_clipboard->Clear();

		BString clipStr;
		clipStr.SetToFormat("%li", field->Value());
		be_clipboard->Data()->AddData("text/plain", B_MIME_TYPE, clipStr.String(), clipStr.Length());
		be_clipboard->Commit();
		be_clipboard->Unlock();
	}


	virtual void MessageReceived(BMessage* message)
	{
		switch (message->what) {
			case '_UKD':
				_AddKeyRow(Window()->CurrentMessage());
				break;
			default:
				BView::MessageReceived(message);
		}
	}


	virtual void KeyDown(const char* bytes, int32 numBytes)
	{
		_AddKeyRow(Window()->CurrentMessage());
		BView::KeyDown(bytes, numBytes);
	}


	virtual void KeyUp(const char* bytes, int32 numBytes)
	{
		_AddKeyRow(Window()->CurrentMessage());
		BView::KeyUp(bytes, numBytes);
	}

private:
	void _AddKeyRow(BMessage* message)
	{
		message->PrintToStream();

		BString eventStr;
		switch(message->what) {
			case '_KYU':
				// utf-8 up arrow U+2191
				eventStr = "\xE2\x86\x91";
				break;
			case '_KYD':
				// utf-8 down arrow U+2193
				eventStr = "\xE2\x86\x93";
				break;
			case '_UKD':
				// unknown key down
				eventStr = "?\xE2\x86\x93";
				break;
		};

		BString modStr;
		int32 modifiers = message->FindInt32("modifiers");
		if (modifiers & B_LEFT_SHIFT_KEY)
			modStr << "LShift";

		if (modifiers & B_RIGHT_SHIFT_KEY)
			modStr << (modStr.Length() > 0 ? "|RShift" : "RShift");

		if (modifiers & B_LEFT_COMMAND_KEY)
			modStr << (modStr.Length() > 0 ? "|LCmd" : "LCmd");

		if (modifiers & B_RIGHT_COMMAND_KEY)
			modStr << (modStr.Length() > 0 ? "|RCmd" : "RCmd");

		if (modifiers & B_LEFT_CONTROL_KEY)
			modStr << (modStr.Length() > 0 ? "|LCtrl" : "LCtrl");

		if (modifiers & B_RIGHT_CONTROL_KEY)
			modStr << (modStr.Length() > 0 ? "|RCtrl" : "RCtrl");

		if (modifiers & B_LEFT_OPTION_KEY)
			modStr << (modStr.Length() > 0 ? "|LOpt" : "LOpt");

		if (modifiers & B_RIGHT_OPTION_KEY)
			modStr << (modStr.Length() > 0 ? "|ROpt" : "ROpt");

		if (modifiers & B_CAPS_LOCK)
			modStr << (modStr.Length() > 0 ? "|Caps" : "Caps");

		if (modifiers & B_SCROLL_LOCK)
			modStr << (modStr.Length() > 0 ? "|Scroll" : "Scroll");

		if (modifiers & B_NUM_LOCK)
			modStr << (modStr.Length() > 0 ? "|Num" : "Num");

		if (modifiers & B_MENU_KEY)
			modStr << (modStr.Length() > 0 ? "|Menu" : "Menu");

		BRow* row = new (std::nothrow) BRow();
		row->SetField(new (std::nothrow) BStringField(eventStr), 0);
		row->SetField(new (std::nothrow) BIntegerField(message->FindInt32("key")), 1);
		row->SetField(new (std::nothrow) BStringField(modStr), 2);
		row->SetField(new (std::nothrow) BIntegerField(message->GetUInt8("states", 0)), 3);
		row->SetField(new (std::nothrow) BIntegerField(message->FindInt8("byte")), 4);
		row->SetField(new (std::nothrow) BStringField(message->FindString("bytes")), 5);
		row->SetField(new (std::nothrow) BIntegerField(message->FindInt32("raw_char")), 6);
		int32 repeat = message->GetInt32("be:key_repeat", -1);
		if (repeat != -1)
			row->SetField(new (std::nothrow) BIntegerField(repeat), 7);

		AddRow(row);
		ScrollTo(row);
	}
};


class KeyTestApp : public BApplication {
public:
	KeyTestApp()
		:
		BApplication("application/x-vnd.cpr.KeyTest")
	{
		BWindow* win = new (std::nothrow) BWindow(BRect(0, 0, 650, 500), "KeyTest", B_TITLED_WINDOW,
			B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE | B_AUTO_UPDATE_SIZE_LIMITS);

		BLayoutBuilder::Group<>(win, B_VERTICAL, 0)
			.Add(new (std::nothrow) KeyListView());

		win->Lock();
		win->CenterOnScreen();
		win->Show();
		win->Unlock();
	}
};


int
main(int /*argc*/, char** /*argv*/)
{
	KeyTestApp app;
	app.Run();

	return 0;
}
