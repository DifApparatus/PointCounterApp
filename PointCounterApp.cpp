#include "PointCounterApp.h"
#include <wx/display.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


PointCounterApp::PointCounterApp(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize())
{
    this->SetSize(horizontalRes,verticalRes);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    wxPanel *panel = new wxPanel(this);
    textctrl = new wxTextCtrl(panel, -1, wxT(""), wxDefaultPosition,
        wxSize(100, 52), wxTE_MULTILINE);
    vbox->Add(panel);

    char bits[bitmapHor*bitmapVert/8];
    GenerateBitmapField(bits,bitmapHor*bitmapVert, desiredPointQuantity);
    wxBitmap bmp(bits,bitmapHor,bitmapVert);
    sb = new wxStaticBitmap(this,-1,bmp);
    sb->Bind(wxEVT_LEFT_DOWN, &PointCounterApp::OnLeftPressed, this);
    sb->Bind(wxEVT_MOTION, &PointCounterApp::onMouseMoved, this);
    sb->Bind(wxEVT_PAINT, &PointCounterApp::OnPaint, this);
    vbox->Add(sb);

    CreateLocusMatrix(bits);

    Centre();
    this->SetSizerAndFit(vbox);
}

void PointCounterApp::GenerateBitmapField(char bits[], int pixelQuantity, int const desiredPointQuantity)
{
    double prob = desiredPointQuantity/static_cast<double>(pixelQuantity);
    pointQuantity = 0;
    srand(time(NULL));
    for (int i=0;i<pixelQuantity/8;i++){
        bits[i] = 0;
        for (int j=1;j < 256;j*=2) {
            double r = rand() / static_cast<double>(RAND_MAX);
            if (r <= prob) {
                bits[i]+=j;
                pointQuantity++;
            }
        }
    }
    return;
}
void PointCounterApp::CreateLocusMatrix(char bitmapBits[])
{
    std::vector<int> vertPointCoords;
    vertPointCoords.reserve(pointQuantity);
    for (int i=0;i<bitmapVert;i++){
        for (int j=0;j<bitmapHor/8;j++){
            if (bitmapBits[i*bitmapHor/8+j]!=0){
                vertPointCoords.push_back(i);
                break;
            }
        }
    }
    std::vector<int> horPointCoords;
    horPointCoords.reserve(pointQuantity);
    for (int j=0;j<bitmapHor/8;j++){
        for (int z=1;z<=128;z*=2){
            for (int i=0;i<bitmapVert;i++){
                if ((bitmapBits[i*bitmapHor/8+j] & z) != 0){
                    horPointCoords.push_back(j*8+log2(z));
                    break;
                }
            }
        }
    }
    std::vector<wxPoint> pointCoords;
    pointCoords.reserve(pointQuantity);
    for (int i=0;i<bitmapVert;i++){
        for (int j=0;j<bitmapHor/8;j++){
            for (int z=1;z<=128;z*=2){
                if ((bitmapBits[i*bitmapHor/8+j] & z) != 0){
                    pointCoords.push_back(wxPoint(j*8+log2(z),i));
                }
            }
        }
    }

    locusRows = vertPointCoords.size() + 1;
    locusCols = horPointCoords.size() + 1;
    locusMatrix = new locus*[locusRows];
    for (int i=0;i<locusRows;i++)
        locusMatrix[i] = new locus[locusCols];

    int pointCounter = 0;
    for (int i=0;i<locusRows;i++){
        int pointCounterInRow = 0;
        for (int j=0;j<locusCols;j++){
            if (i==0){
                locusMatrix[i][j].y_min = INT_MIN;
                locusMatrix[i][j].y_max = vertPointCoords[i];
            }else if (i == locusRows-1){
                locusMatrix[i][j].y_min = vertPointCoords[i-1];
                locusMatrix[i][j].y_max = INT_MAX;
            }else {
                locusMatrix[i][j].y_min = vertPointCoords[i-1];
                locusMatrix[i][j].y_max = vertPointCoords[i];
            }

            if (j==0){
                locusMatrix[i][j].x_min = INT_MIN;
                locusMatrix[i][j].x_max = horPointCoords[j];
            }else if (j == locusCols-1){
                locusMatrix[i][j].x_min = horPointCoords[j-1];
                locusMatrix[i][j].x_max = INT_MAX;
            }else {
                locusMatrix[i][j].x_min = horPointCoords[j-1];
                locusMatrix[i][j].x_max = horPointCoords[j];
            }
            if (i != 0) { locusMatrix[i][j].Q = locusMatrix[i-1][j].Q + pointCounterInRow; }
            else { locusMatrix[i][j].Q = pointCounterInRow; }
            if (locusMatrix[i][j].y_min == pointCoords[pointCounter].y && locusMatrix[i][j].x_max == pointCoords[pointCounter].x){
                pointCounterInRow++;
                pointCounter++;
            }
        }
    }
}

int PointCounterApp::CalculatePoints(int x_min, int x_max, int y_min, int y_max)
{
    int* xArray = new int[locusCols];
    for (int i = 0; i < locusCols; i++)
        xArray[i] = locusMatrix[0][i].x_min;
    int x_min_index = binarySearch(xArray, 0, locusCols-1, x_min);
    int x_max_index = binarySearch(xArray, 0, locusCols-1, x_max);

    int* yArray = new int[locusRows];
    for (int i = 0; i < locusRows; i++)
        yArray[i] = locusMatrix[i][0].y_min;
    int y_min_index = binarySearch(yArray, 0, locusRows-1, y_min);
    int y_max_index = binarySearch(yArray, 0, locusRows-1, y_max);

    int Q1 = locusMatrix[y_max_index][x_max_index].Q;
    int Q2 = locusMatrix[y_max_index][x_min_index].Q;
    int Q3 = locusMatrix[y_min_index][x_max_index].Q;
    int Q4 = locusMatrix[y_min_index][x_min_index].Q;
    return Q1-Q2-Q3+Q4;
}

void PointCounterApp::OnLeftPressed(wxMouseEvent& event)
{
    wxPoint point = wxGetMousePosition();
    point.y = bitmapVert + point.y - verticalRes;
    int min_x = std::min(startPoint.x, endPoint.x);
    int max_x = std::max(startPoint.x, endPoint.x);
    int min_y = std::min(startPoint.y, endPoint.y);
    int max_y = std::max(startPoint.y, endPoint.y);
    if (point.x < max_x && point.x > min_x && point.y < max_y && point.y > min_y){
        regionSelected = 1;
        priorDragPoint = wxGetMousePosition();
   }
   else {
        regionSelected = 0;
        startPoint = wxGetMousePosition();
        startPoint.y = bitmapVert + startPoint.y - verticalRes;
   }
}

void PointCounterApp::onMouseMoved(wxMouseEvent& event)
{
    if (event.Dragging()){
        if (regionSelected){
            wxPoint curDragPoint = wxGetMousePosition();
            wxPoint dPoint = curDragPoint - priorDragPoint;
            priorDragPoint = curDragPoint;
            endPoint = endPoint + dPoint;
            startPoint = startPoint + dPoint;
            int pointsNumber = CalculatePoints(std::min(startPoint.x, endPoint.x),std::max(startPoint.x, endPoint.x),
                            std::min(startPoint.y, endPoint.y),std::max(startPoint.y, endPoint.y) );
            wxString s;
            textctrl->ChangeValue(s << pointsNumber);
            sb->Refresh();
            sb->Update();
            wxClientDC dc(sb);
            dc.SetLogicalOrigin(0,-52);
            dc.SetPen(*wxBLACK_PEN);
            dc.SetBrush( *wxTRANSPARENT_BRUSH);
            int min_x = std::min(startPoint.x, endPoint.x);
            int max_x = std::max(startPoint.x, endPoint.x);
            int min_y = std::min(startPoint.y, endPoint.y);
            int max_y = std::max(startPoint.y, endPoint.y);
            wxRect recToDraw(wxPoint(max_x, max_y),wxPoint(min_x,min_y));
            dc.DrawRectangle(recToDraw);
        }
        else {
            endPoint = wxGetMousePosition();
            int deltaY = bitmapVert - verticalRes;
            endPoint.y = endPoint.y + deltaY;

            int pointsNumber = CalculatePoints(std::min(startPoint.x, endPoint.x),std::max(startPoint.x, endPoint.x),
                            std::min(startPoint.y, endPoint.y),std::max(startPoint.y, endPoint.y) );
            wxString s;
            textctrl->ChangeValue(s << pointsNumber);
            sb->Refresh();
            sb->Update();
            wxClientDC dc(sb);
            dc.SetLogicalOrigin(0,-52);
            dc.SetPen(*wxBLACK_PEN);
            dc.SetBrush( *wxTRANSPARENT_BRUSH);
            int min_x = std::min(startPoint.x, endPoint.x);
            int max_x = std::max(startPoint.x, endPoint.x);
            int min_y = std::min(startPoint.y, endPoint.y);
            int max_y = std::max(startPoint.y, endPoint.y);
            wxRect recToDraw(wxPoint(max_x, max_y),wxPoint(min_x,min_y));
            dc.DrawRectangle(recToDraw);
        }
    }
}

void PointCounterApp::OnPaint(wxPaintEvent& event)
{
}

// Search via minimum
int binarySearch(int* arr, int l, int r, int x)
{
    if ( r - l <= 1){
        if (arr[l] == INT_MIN || arr[r] == INT_MAX ) return r; //Border
        else return l; //Not border
    }
    else {
        int mid = l + (r - l) / 2;
        if (arr[mid] > x) return binarySearch(arr, l, mid, x);
        if (arr[mid] < x) return binarySearch(arr, mid, r, x);
        else return binarySearch(arr,mid, mid+1, x); // x == arr[mid]
    }
 }
