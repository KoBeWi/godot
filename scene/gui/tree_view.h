/**************************************************************************/
/*  tree_view.h                                                           */
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

#include "scene/gui/control.h"

class HScrollBar;
class VScrollBar;
class TreeViewItem;

class TreeView : public Control {
	GDCLASS(TreeView, Control);

	friend class TreeViewItem;
	friend class TreeViewCell;

	struct Column {
		int custom_min_width = 0;
		int expand_ratio = 1;
		bool expand = true;
		bool clip_content = false;
		String title_tooltip;
		String title;
		String xl_title;
		HorizontalAlignment title_alignment = HORIZONTAL_ALIGNMENT_CENTER;
		Ref<TextParagraph> text_buf;
		String language;
		Control::TextDirection text_direction = Control::TEXT_DIRECTION_INHERITED;

		mutable int cached_minimum_width = 0;
		mutable bool cached_minimum_width_dirty = true;
	};

	LocalVector<Column> columns;

	// TODO: Default cell type

	TreeViewItem *root = nullptr;
	bool hide_root = false;

	HScrollBar *h_scroll = nullptr;
	VScrollBar *v_scroll = nullptr;
	Vector2 scroll_offset;

	Rect2 _get_content_rect() const;

	struct ThemeCache {
		Ref<StyleBox> panel;

		Color guide_color;

		int item_margin = 0;
		int h_separation = 0;
		bool draw_guides = false;
	} theme_cache;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	TreeViewItem *create_item(TreeViewItem *p_parent = nullptr, int p_index = -1);

	int get_column_minimum_width(int p_column) const;

	TreeView();
};
