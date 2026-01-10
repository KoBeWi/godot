/**************************************************************************/
/*  tree_view.cpp                                                         */
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

#include "tree_view.h"

#include "scene/gui/scroll_bar.h"
#include "scene/gui/tree_view_item.h"

Rect2 TreeView::_get_content_rect() const {
	const Size2 control_size = get_size();
	const Ref<StyleBox> background = theme_cache.panel;

	// This is the background stylebox's content rect.
	const real_t width = control_size.x - background->get_margin(SIDE_LEFT) - background->get_margin(SIDE_RIGHT);
	const real_t height = control_size.y - background->get_margin(SIDE_TOP) - background->get_margin(SIDE_BOTTOM);
	const Rect2 content_rect = Rect2(background->get_offset(), Size2(width, height));

	// Scrollbars won't affect Tree's content rect if they're not visible or placed inside the stylebox margin area.
	// const real_t v_size = v_scroll->is_visible() ? (v_scroll->get_combined_minimum_size().x + theme_cache.scrollbar_h_separation) : 0;
	// const real_t h_size = h_scroll->is_visible() ? (h_scroll->get_combined_minimum_size().y + theme_cache.scrollbar_v_separation) : 0;
	// const Point2 scroll_begin = _get_scrollbar_layout_rect().get_end() - Vector2(v_size, h_size);
	// const Size2 offset = (content_rect.get_end() - scroll_begin).maxf(0);

	// return content_rect.grow_individual(0, 0, -offset.x, -offset.y);
	return content_rect;
}

void TreeView::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			draw_style_box(theme_cache.panel, Rect2(Vector2(), get_size()));

			if (root) {
				root->_draw(_get_content_rect());
			}
		} break;
	}
}

void TreeView::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_item", "parent", "index"), &TreeView::create_item, DEFVAL(Variant()), DEFVAL(-1));

	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, TreeView, panel);

	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, TreeView, guide_color);

	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, TreeView, draw_guides);
}

TreeViewItem *TreeView::create_item(TreeViewItem *p_parent, int p_index) {
	TreeViewItem *new_item = nullptr;
	if (p_parent) {
		ERR_FAIL_COND_V_MSG(p_parent->tree != this, nullptr, "A different tree owns the given parent");
		new_item = p_parent->create_child(p_index);
	} else {
		if (!root) {
			new_item = memnew(TreeViewItem(this));
			root = new_item;
		} else {
			new_item = root->create_child(p_index);
		}
	}
	return new_item;
}

int TreeView::get_column_minimum_width(int p_column) const {
	ERR_FAIL_INDEX_V(p_column, (int)columns.size(), -1);

	if (columns[p_column].cached_minimum_width_dirty) {
		// Use the custom minimum width.
		int min_width = columns[p_column].custom_min_width;

		// Check if the visible title of the column is wider.
		// if (show_column_titles) {
		// 	const float padding = theme_cache.title_button->get_margin(SIDE_LEFT) + theme_cache.title_button->get_margin(SIDE_RIGHT);
		// 	min_width = MAX(theme_cache.font->get_string_size(columns[p_column].xl_title, HORIZONTAL_ALIGNMENT_LEFT, -1, theme_cache.font_size).width + padding, min_width);
		// }

		if (root && !columns[p_column].clip_content) {
			int depth = 1;

			TreeViewItem *last = nullptr;
			TreeViewItem *first = hide_root ? root->get_next_visible() : root;
			for (TreeViewItem *item = first; item; last = item, item = item->get_next_visible()) {
				// Get column indentation.
				int indent;
				if (p_column == 0) {
					if (last) {
						if (item->parent == last) {
							depth += 1;
						} else if (item->parent != last->parent) {
							depth = hide_root ? 0 : 1;
							for (TreeViewItem *iter = item->parent; iter; iter = iter->parent) {
								depth += 1;
							}
						}
					}
					indent = theme_cache.item_margin * depth;
				} else {
					indent = theme_cache.h_separation;
				}

				// Get the item minimum size.
				Size2 item_size = item->get_minimum_size(p_column);
				item_size.width += indent;

				// Check if the item is wider.
				min_width = MAX(min_width, item_size.width);
			}
		}

		columns[p_column].cached_minimum_width = min_width;
		columns[p_column].cached_minimum_width_dirty = false;
	}

	return columns[p_column].cached_minimum_width;
}

TreeView::TreeView() {
	// h_scroll = memnew(HScrollBar);
	// add_child(h_scroll);

	// v_scroll = memnew(VScrollBar);
	// add_child(v_scroll);
}
