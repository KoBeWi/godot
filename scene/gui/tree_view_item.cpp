/**************************************************************************/
/*  tree_view_item.cpp                                                    */
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

#include "tree_view_item.h"

#include "scene/gui/tree_view.h"
#include "scene/gui/tree_view_cell.h"

TreeViewItem *TreeViewItem::_get_next_in_tree(bool p_wrap, bool p_include_invisible) {
	TreeViewItem *current = this;

	if ((!current->collapsed || p_include_invisible) && current->get_first_child()) {
		current = current->get_first_child();
	} else if (current->next) {
		current = current->next;
	} else {
		while (current && !current->next) {
			current = current->parent;
		}

		if (!current) {
			if (p_wrap) {
				return tree->root;
			} else {
				return nullptr;
			}
		} else {
			current = current->next;
		}
	}

	return current;
}

void TreeViewItem::_bind_methods() {
}

void TreeViewItem::set_visible(bool p_visible) {
	// TODO
}

void TreeViewItem::_draw(const Rect2 &p_rect) {
	if (!is_visible_in_tree()) {
		return;
	}

	const real_t bottom_margin = tree->theme_cache.panel->get_margin(SIDE_BOTTOM); // Extra stylebox space below the content.
	const real_t draw_height = p_rect.size.y + bottom_margin; // Visible height including bottom margin.

	// Cull item if it's beyond the bottom of the visible area.
	if (p_rect.position.y - tree->scroll_offset.y > draw_height) {
		return;
	}

	bool skip = (tree->hide_root && tree->root == this);

	int i = -1;
	for (const TreeViewCell *cell : cells) {
		i++;

		Rect2 cell_rect = Rect2(p_rect.position, Vector2(tree->get_column_minimum_width(i), 10));
		cell->_draw(cell_rect);
	}

	Rect2 rect(p_rect.position, Vector2(100, 20));
}

TreeViewItem *TreeViewItem::get_next_visible(bool p_wrap) {
	TreeViewItem *loop = this;
	TreeViewItem *next_item = _get_next_in_tree(p_wrap);
	while (next_item && !next_item->is_visible_in_tree()) {
		next_item = next_item->_get_next_in_tree(p_wrap);
		if (next_item == loop) {
			// Check that we haven't looped all the way around to the start.
			next_item = nullptr;
			break;
		}
	}
	return next_item;
}

TreeViewItem *TreeViewItem::create_child(int p_index) {
	return memnew(TreeViewItem(tree));
}

Size2 TreeViewItem::get_minimum_size(int p_column) {
	return Size2();
}

TreeViewItem::TreeViewItem(TreeView *p_tree) {
	tree = p_tree;

	for (int i = 0; i < (int)tree->columns.size(); i++) {
		cells.push_back(memnew(TreeViewCellText(this, i)));
	}
}
