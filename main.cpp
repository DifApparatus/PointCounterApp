#include "main.h"
#include "PointCounterApp.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    PointCounterApp *app = new PointCounterApp(wxT("PointCounterApp"));
    app->Show(true);

    return true;
}
