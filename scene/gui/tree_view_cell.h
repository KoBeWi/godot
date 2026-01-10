/**************************************************************************/
/*  tree_view_cell.h                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/object/class_db.h"

class Texture2D;
class TreeViewItem;

class TreeViewCell : public Object {
	GDCLASS(TreeViewCell, Object);

	friend class TreeViewItem;

protected:
	TreeViewItem *tree_item = nullptr;
	TreeView *tree = nullptr;

	struct CellButton {
		int id = 0;
		bool disabled = false;
		Ref<Texture2D> texture;
		Color color = Color(1, 1, 1, 1);
		String tooltip;
	};
	LocalVector<CellButton> buttons;

	virtual void _draw(const Rect2 &p_rect) const;

	static void _bind_methods();

public:
	// virtual void update() = 0;
	virtual int get_height();
	// virtual void draw(Point2i p_pos, int p_label_h, Point2 p_draw_ofs, int &r_skip, int &r_offset);

	void add_button(const Ref<Texture2D> &p_button, int p_id = -1, bool p_disabled = false, const String &p_tooltip = "");

	Size2 get_icon_size() const;
	void draw_icon(const RID &p_where, const Point2 &p_pos, const Size2 &p_size = Size2(), const Color &p_color = Color()) const;

	TreeViewCell(TreeViewItem *p_item, int p_column);
};

class TreeViewCellText : public TreeViewCell {
	GDCLASS(TreeViewCellText, TreeViewCell);

protected:
	static void _bind_methods();

public:
	// virtual void update() override;
	// virtual void draw(Point2i p_pos, int p_label_h, Point2 p_draw_ofs, int &r_skip, int &r_offset) override;

	TreeViewCellText(TreeViewItem *p_item, int p_column) :
			TreeViewCell(p_item, p_column) {}
};

class TreeViewCellCheck : public TreeViewCellText {
	GDCLASS(TreeViewCellCheck, TreeViewCellText);

public:
	// virtual void update() override {}
	// virtual void draw(Point2i p_pos, int p_label_h, Point2 p_draw_ofs, int &r_skip, int &r_offset) override {}
};

class TreeViewCellRange : public TreeViewCell {
	GDCLASS(TreeViewCellRange, TreeViewCell);

public:
	// virtual void update() override {}
	// virtual void draw(Point2i p_pos, int p_label_h, Point2 p_draw_ofs, int &r_skip, int &r_offset) override {}
};

class TreeViewCellCombo : public TreeViewCell {
	GDCLASS(TreeViewCellCombo, TreeViewCell);

public:
	// virtual void update() override {}
	// virtual void draw(Point2i p_pos, int p_label_h, Point2 p_draw_ofs, int &r_skip, int &r_offset) override {}
};
