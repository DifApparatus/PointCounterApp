#include <wx/wx.h>
#include <wx/event.h>

struct locus{
    int x_min;
    int x_max;
    int y_min;
    int y_max;
    int Q; //Dominance number
};

class PointCounterApp : public wxFrame
{
public:
    PointCounterApp(const wxString& title);

private:
    int const horizontalRes = 1920;
    int const verticalRes = 1080;
    wxTextCtrl *textctrl;
    wxStaticBitmap *sb;
    //bitmap resolution should be divisible by 8
    int const bitmapHor = 1920;
    int const bitmapVert = 1000;

    int const desiredPointQuantity = 1000000;
    int pointQuantity;

    locus** locusMatrix;
    int locusRows;
    int locusCols;

    bool regionSelected = 0;
    wxPoint priorDragPoint;

    wxPoint startPoint;
    wxPoint endPoint;
    void GenerateBitmapField(char bits[], int pixelQuantity, int desiredPointQuantity);
    void CreateLocusMatrix(char bitmapBits[]);
    int CalculatePoints(int x_min, int x_max, int y_min, int y_max);

    void OnLeftPressed(wxMouseEvent& event);
    void onMouseMoved(wxMouseEvent& event);
    void OnPaint(wxPaintEvent& event);

};

//Search via minimum
int binarySearch(int* arr, int l, int r, int x);
