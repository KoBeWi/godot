/**************************************************************************/
/*  tree_view_item.h                                                      */
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

class TreeView;
class TreeViewCell;

class TreeViewItem : public Object {
	GDCLASS(TreeViewItem, Object);

	friend class TreeView;
	friend class TreeViewCell;

	bool visible = true;
	bool parent_visible_in_tree = true;
	bool collapsed = false;

	TreeView *tree = nullptr;
	TreeViewItem *parent = nullptr;
	TreeViewItem *next = nullptr;
	LocalVector<TreeViewItem *> children;

	LocalVector<TreeViewCell *> cells;

	TreeViewItem *_get_next_in_tree(bool p_wrap = false, bool p_include_invisible = false);

protected:
	static void _bind_methods();

	virtual void _draw(const Rect2 &p_rect);

public:
	void set_visible(bool p_visible);
	bool is_visible() const { return visible; }
	bool is_visible_in_tree() const { return visible && parent_visible_in_tree; }

	TreeViewItem *get_first_child() { return children.is_empty() ? nullptr : children[0]; }
	TreeViewItem *get_next_visible(bool p_wrap = false);

	TreeViewItem *create_child(int p_index = -1);

	Size2 get_minimum_size(int p_column);

	TreeViewItem(TreeView *p_tree);
};
