#include "imgui_extend/table_view.h"
#include <algorithm>

namespace ImGui::extend
{
    void
    TableView::draw(ImVec2 const& outer_size)
    {
        ImGui::PushID(this);

        if (ImGui::BeginChild("##TableView", outer_size))
        {
            int columns = columnCount();
            if (ImGui::BeginTable("##TableView", columns, tableFlags(), outer_size))
            {
                for (int i = 0; i < columns; ++i)
                {
                    ImGui::TableSetupColumn(headerLable(i).c_str(), headerFlags(i), headerInitWidthOrWeight(i));
                }
                ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
                ImGui::TableHeadersRow();

                if (reloadRows_)
                {
                    generateDrawRowIndexes();
                    sortDrawRowIndexes();
                    reloadRows_ = false;
                }

                sortDrawRowIndexes();

                ImGuiListClipper clipper_;
                clipper_.Begin(drawRowIndexes_.size());
                while (clipper_.Step())
                {
                    for (int row_n = clipper_.DisplayStart; row_n < clipper_.DisplayEnd; row_n++)
                    {
                        ImGui::TableNextRow();

                        auto const& draw_row = drawRowIndexes_[row_n];
                        ImGui::PushID(draw_row);

                        float row_max_height = 0;
                        for (int column = 0; column < columns; ++column)
                        {
                            ImGui::TableSetColumnIndex(column);

                            ImGui::PushID(column);

                            ImGui::BeginGroup();
                            drawCell(draw_row, column);
                            ImGui::EndGroup();

                            row_max_height = std::max(row_max_height, ImGui::GetItemRectSize().y);

                            ImGui::PopID();
                        }
                        ImGui::SameLine();

                        bool const item_is_selected = selection_.contains(draw_row);
                        if (ImGui::Selectable("##row_select",
                                              item_is_selected,
                                              ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
                                              ImVec2(0, row_max_height)))
                        {
                            if (ImGui::GetIO().KeyCtrl)
                            {
                                if (item_is_selected)
                                {
                                    selection_.find_erase_unsorted(draw_row);
                                }
                                else
                                {
                                    selection_.push_back(draw_row);
                                }
                            }
                            else
                            {
                                selection_.clear();
                                selection_.push_back(draw_row);
                            }
                        }

                        ImGui::PopID();
                    }
                }
                clipper_.End();

                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
        ImGui::PopID();
    }

    ImGuiTableFlags
    TableView::tableFlags() const
    {
        static ImGuiTableFlags flags = ImGuiTableFlags_SortMulti | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable
                                       | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_Reorderable
                                       | ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders
                                       | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollX
                                       | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
        return flags;
    }

    ImGuiTableColumnFlags
    TableView::headerFlags(int column) const
    {
        return 0;
    }

    float
    TableView::headerInitWidthOrWeight(int column) const
    {
        return 0.f;
    }

    bool
    TableView::rowPassFilter(int row) const
    {
        return true;
    }

    int
    TableView::rowCompare(int left_row, int right_row, int column) const
    {
        (void)column;
        return left_row - right_row;
    }

    void
    TableView::generateDrawRowIndexes()
    {
        int rows = rowCount();

        drawRowIndexes_.clear();
        for (int row = 0; row < rows; ++row)
        {
            if (rowPassFilter(row))
            {
                drawRowIndexes_.push_back(row);
            }
        }
    }
    void
    TableView::sortDrawRowIndexes()
    {
        if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
        {
            if (reloadRows_)
            {
                sort_specs->SpecsDirty = true;
            }

            if (sort_specs->SpecsDirty)
            {
                std::sort(drawRowIndexes_.begin(),
                          drawRowIndexes_.end(),
                          [&](int const& left_row, int const& right_row)
                          {
                              for (int n = 0; n < sort_specs->SpecsCount; n++)
                              {
                                  ImGuiTableColumnSortSpecs const& spec = sort_specs->Specs[n];

                                  int delta = rowCompare(left_row, right_row, spec.ColumnIndex);

                                  if (delta > 0)
                                  {
                                      return (spec.SortDirection == ImGuiSortDirection_Descending);
                                  }
                                  if (delta < 0)
                                  {
                                      return (spec.SortDirection == ImGuiSortDirection_Ascending);
                                  }
                              }
                              return left_row < right_row;
                          });
                sort_specs->SpecsDirty = false;
            }
        }
    }

    std::vector<int> const&
    TableView::getDrawRowIndexes() const
    {
        return drawRowIndexes_;
    }

    void
    TableView::tableReloadRows()
    {
        reloadRows_ = true;
    }

} // namespace ImGui::extend