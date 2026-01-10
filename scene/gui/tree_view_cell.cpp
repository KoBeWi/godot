/**************************************************************************/
/*  tree_view_cell.cpp                                                    */
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

#include "tree_view_cell.h"

#include "scene/gui/tree_view.h"
#include "scene/gui/tree_view_item.h"
#include "scene/resources/texture.h"

void TreeViewCell::_draw(const Rect2 &p_rect) const {
	if (tree->theme_cache.draw_guides) {
		RenderingServer::get_singleton()->canvas_item_add_line(tree->get_canvas_item(), Point2i(p_rect.position.x, p_rect.position.y + p_rect.size.y), p_rect.position + p_rect.size, tree->theme_cache.guide_color, 1);
	}
}

void TreeViewCell::_bind_methods() {
}

int TreeViewCell::get_height() {
	return 0;
}

void TreeViewCellText::_bind_methods() {
	// ClassDB::bind_method(D_METHOD("set_text", "text"), &TreeItemCellText::set_text);
	// ClassDB::bind_method(D_METHOD("get_text"), &TreeItemCellText::get_text);

	// ADD_PROPERTY(PropertyInfo(Variant::INT, "text"), "set_text", "get_text");
}

TreeViewCell::TreeViewCell(TreeViewItem *p_item, int p_column) {
	tree = p_item->tree;
	tree_item = p_item;
}
