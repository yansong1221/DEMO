#pragma once
#include "export.h"
#include "imgui.h"
#include <string>
#include <vector>

namespace ImGui::extend
{
    class IMGUI_EXTEND_API TableView
    {
      public:
        virtual ~TableView() = default;

        virtual int columnCount() const = 0;
        virtual ImGuiTableFlags tableFlags() const;

        virtual std::string headerLable(int column) const = 0;
        virtual ImGuiTableColumnFlags headerFlags(int column) const;
        virtual float headerInitWidthOrWeight(int column) const;

        virtual int rowCount() const = 0;
        virtual bool rowPassFilter(int row) const;
        virtual int rowCompare(int left_row, int right_row, int column) const;

        std::vector<int> const& getDrawRowIndexes() const;

      public:
        virtual void draw(ImVec2 const& outer_size = ImVec2(0.0f, 0.0f));

      protected:
        virtual void drawCell(int row, int column) = 0;

        void tableReloadRows();

      private:
        void generateDrawRowIndexes();
        void sortDrawRowIndexes();

      private:
        std::vector<int> drawRowIndexes_;
        bool reloadRows_ = true;
        ImVector<int> selection_;
    };
} // namespace ImGui::extend