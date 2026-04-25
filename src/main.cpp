#include "app/Application.hpp"
#include "factory/PrimitiveFactory.hpp"

#include <iostream>

namespace
{

void onCreateNewDocument(Application& app)
{
    std::cout << "[GUI]\tCreate new document\n";
    app.createNewDocument();
}

void onImportDocument(Application& app)
{
    std::cout << "[GUI]\tImport document\n";
    app.importDocument("output.editor");
}

void onExportDocument(Application& app)
{
    std::cout << "[GUI]\tExport document\n";
    app.exportDocument("output.editor");
}

void onCreateRectangle(Application& app)
{
    std::cout << "[GUI]\tCreate rectangle\n";
    app.createRectangle();
}

void onCreateCircle(Application& app)
{
    std::cout << "[GUI]\tCreate circle\n";
    app.createCircle();
}

void onCreateLine(Application& app)
{
    std::cout << "[GUI]\tCreate line\n";
    app.createLine();
}

void onDeletePrimitive(Application& app)
{
    std::cout << "[GUI]\tDelete primitive\n";
    app.deletePrimitive(0);
}

} // namespace

int main()
{
    Application app;

    std::cout << "\n--------------------------------------------------\n";
    std::cout << "Scenario 1: create new document\n";
    onCreateNewDocument(app);
    onCreateRectangle(app);
    onCreateCircle(app);
    onCreateLine(app);
    onExportDocument(app);
    
    std::cout << "\n--------------------------------------------------\n";
    std::cout << "Scenario 2: export, clear and import document\n";
    onExportDocument(app);

    std::cout << "[GUI]\tClear document\n";
    app.createNewDocument();

    std::cout << "[GUI]\tCurrent document after clear\n";
    app.render();

    onImportDocument(app);

    std::cout << "[GUI]\tCurrent document after import\n";
    app.render();

    return 0;
}