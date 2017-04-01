#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <ctime>
#include <climits>
using namespace std;

/*
Règle 1 Regrouper chaque couleur
Règle 2 Deux cases séparées par une couleur peuvent se regrouper
Règle 3 Pour déplacer plusieurs cases, faites glisser au niveau de la case qui traversera la couleur
Règle 4 Un groupe de cases peut se former tant qu'il ne détruit pas des groupes plus petits
*/

/*
0. Numérotation des formes, 1 numéro par forme connexe [algo récursif]
    - une forme stocke
            - une couleur
            - 4 déplacements absolus max (+x,-x,+y,-y) valables pour toutes les cases

1. Préparer le jeu
    - Déterminer les cases jouables et les directions jouables [algo edge]
    - Pour chaque case
        - déterminer s'il est possible de jouer
            - si réponse = ok alors -> point 2
            - si réponse = ko alors (animation vibration)
2. Jouer
    - Dupliquer le tableau de cases
    - Jouer à la position
    - Détermination des nouvelles formes
    - Si nouvelles formes brisent la règle 4 -> (animation groupes)
    - Calcul du nouveau score
        - Enregistrer
    - Préparer le jeu -> point 1
*/

// background colors
#define BG_BLACK "\33[40m"
#define BG_RED "\33[41m"
#define BG_GREEN "\33[42m"
#define BG_YELLOW "\33[43m"
#define BG_BLUE "\33[44m"
#define BG_MAJENTA "\33[45m"
#define BG_CYAN "\33[46m"
#define BG_WHITE "\33[47m"

enum eColor
{
    red,
    green,
    yellow,
    blue,
    majenta,
    cyan,
    white,
    color_none,
    color_max
};
static string s_colors[] = { BG_RED, BG_GREEN, BG_YELLOW, BG_BLUE, BG_MAJENTA, BG_CYAN, BG_WHITE, BG_BLACK };

enum eDirection
{
    direction_left,
    direction_right,
    direction_up,
    direction_down,
    direction_max
};

static string s_directions[] = { "left", "right", "up", "down" };

enum eError
{
    ok,
    error_parameters,
    error_cannot_play
};
static string s_errors[] = { "ok", "error_parameters", "error_cannot_play" };

class Table
{
public:
    Table(int x, int y, int color_nb):_x(x),_y(y),_color_nb(color_nb),_group_nb(0),
        _boxes(new Box[x * y]()),_groups(NULL) { }
    virtual ~Table()
    {
        if (_boxes != NULL)
            delete[] _boxes;
        if (_groups != NULL)
            delete[] _groups;
    }

    // accessors
    int getX() { return _x; }
    int getY() { return _y; }
    int getColors() { return _color_nb; }
    void setColor(int index, eColor color) { _boxes[index].color = color; }

    eError setColors(eColor colors[], int color_nb)
    {
        eError err = ok;

        if (color_nb == _x * _y)
            for(int index = 0; index < _x * _y; index++)
                _boxes[index].color = colors[index];
        else
            err = error_parameters;

        return err;
    }

    // 1. numerate and prepare
    void numerate()
    {
        // set same group id to cases with same color
        _group_nb = 0;
        for(int index = 0; index < _x * _y; index++)
            if (_boxes[index].group == -1)
                numerate_recurse(index, _group_nb++);

        // get groups properties
        // here assert that all cases have a group between 0 and _group_nb-1
        if (_group_nb > 0)
        {
            _groups = new Group[_group_nb]();
            for(int index = 0; index < _x * _y; index++)
            {
                if (_groups[_boxes[index].group].xmin > (index%_x))
                    _groups[_boxes[index].group].xmin = (index%_x);
                if (_groups[_boxes[index].group].ymin > (index/_x))
                    _groups[_boxes[index].group].ymin = (index/_x);
                if (_groups[_boxes[index].group].xmax < (index%_x))
                    _groups[_boxes[index].group].xmax = (index%_x);
                if (_groups[_boxes[index].group].ymax < (index/_x))
                    _groups[_boxes[index].group].ymax = (index/_x);
            }
        }
    }

    // 2. play
    eError play(int xPlay, int yPlay, eDirection direction, int& deplace)
    {
        eError err = error_parameters;

        if ((xPlay >= 0) && (xPlay < _x) && (yPlay >= 0) && (yPlay < _y))
        {
            // get group max deplacement
            int upMax, downMax, leftMax, rightMax;
            getGroupLiberty(_groups[_boxes[xPlay + _x * yPlay].group], upMax, downMax, leftMax, rightMax);

            // calc
            switch(direction)
            {
                case direction_left:
                    err = canPlayLeft(xPlay, yPlay, upMax, downMax, leftMax, rightMax, deplace);
                    break;

                case direction_right:
                    err = canPlayRight(xPlay, yPlay, upMax, downMax, leftMax, rightMax, deplace);
                    break;

                case direction_up:
                    err = canPlayUp(xPlay, yPlay, upMax, downMax, leftMax, rightMax, deplace);
                    break;

                case direction_down:
                    err = canPlayDown(xPlay, yPlay, upMax, downMax, leftMax, rightMax, deplace);
                    break;

                default:
                    err = error_parameters;
            }
        }
        else
            err = error_parameters;

        return err;
    }

    void print()
    {
        cout << "   ";
        for(int x = 0; x < _x; x++)
            cout << setw(2) << x << " ";
        cout << endl << endl;

        for(int y = 0; y < _y; y++)
        {
            cout << setw(2) << y << " ";
            for(int x = 0; x < _x; x++)
            {
                cout << s_colors[_boxes[x+_x*y].color]
                    << setw(2) << _boxes[x+_x*y].group
                    << s_colors[color_none] << " ";
            }
            cout << endl << endl;
        }

        /*for(int group = 0; group < _group_nb; group++)
            cout << "group " << group << " (xmin,ymin,xmax,ymax) = ("
                << _groups[group].xmin << "," << _groups[group].ymin << ","
                << _groups[group].xmax << "," << _groups[group].ymax << ")" << endl;
        */
    }

private:

    //
    struct Box
    {
        Box():color(color_none),group(-1) { }
        eColor color;
        int group;
    };

    struct Group
    {
        Group():xmin(INT_MAX),ymin(INT_MAX),xmax(INT_MIN),ymax(INT_MIN) { }
        int xmin, ymin, xmax, ymax;
    };

    //
    int _x;
    int _y;
    int _color_nb;
    int _group_nb;
    Box* _boxes;
    Group* _groups;

    // play functions
    void numerate_recurse(int index, int group)
    {
        eColor color = _boxes[index].color;
        _boxes[index].group = group;

        if ( ((index%_x) > 0) && (_boxes[index - 1].group == -1) && (_boxes[index - 1].color == color))
            numerate_recurse(index - 1, group); // left
        if ( ((index%_x) < (_x - 1)) && (_boxes[index + 1].group == -1) && (_boxes[index + 1].color == color))
            numerate_recurse(index + 1, group); // right
        if ( ((index/_x) > 0) && (_boxes[index - _x].group == -1) && (_boxes[index - _x].color == color))
            numerate_recurse(index - _x, group); // up
        if ( ((index/_x) < (_y - 1)) && (_boxes[index + _x].group == -1) && (_boxes[index + _x].color == color))
            numerate_recurse(index + _x, group); // down
    }

    void getGroupLiberty(Group& group, int& upMax, int& downMax, int& leftMax, int& rightMax)
    {
        upMax = group.ymin;
        downMax = _y - group.ymax - 1;
        leftMax = group.xmin;
        rightMax = _x - group.xmax - 1;
    }

    eError canPlay(int xPlay, int yPlay, int upMax, int downMax, int leftMax, int rightMax, int& deplace, int xBoundary, int yBoundary, int xDelta, int yDelta)
    {
        eError err = error_cannot_play;
        int x = xPlay + xDelta;
        int y = yPlay + yDelta;

        if (x >= 0 && x < _x && y >= 0 && y < _x)
        {
            // phase 1: can a colored box being dragged through a color toward another box with the same color ?
            eColor colorThrough = _boxes[x + _x * y].color;
            deplace = 0;
            do
            {
                deplace++;
                x += xDelta;
                y += yDelta;
            }
            while(x >= 0 && x < _x && y >= 0 && y < _x && _boxes[x + _x * y].color == colorThrough);

            if (deplace > 0 && (x >= 0) && _boxes[x + _x * yPlay].color == _boxes[xPlay + _x * yPlay].color)
            {
                // phase 1.1: can the dragged box group being dragged ?
                if (deplace <= leftMax)
                    err = ok;
            }

            if (err != ok && deplace > 0 && yPlay < _y - 1)
            {
                // phase 2: can a colored box being dragged through a color toward another LATERAL box with the same color ?
                x = xPlay - 1;
                deplace = 0;
                bool foundLateral = false;
                do
                {
                    deplace++;

                    if (_boxes[x + _x * (yPlay + 1)].color == _boxes[xPlay + _x * yPlay].color)
                    {
                        // found a lateral color matching with play color
                        foundLateral = true;
                        break;
                    }

                    x--;
                }
                while(x >= 0 && _boxes[x + _x * yPlay].color == colorThrough);

                if (deplace > 0 && (x >= 0) && foundLateral)
                {
                    // phase 1.1: can the dragged box group being dragged ?
                    if (deplace <= leftMax)
                        err = ok;
                }
            }

            if (err != ok && deplace > 0 && yPlay > 0)
            {
                // phase 2.1: can a colored box being dragged through a color toward another LATERAL box with the same color ?
                x = xPlay - 1;
                deplace = 0;
                bool foundLateral = false;
                do
                {
                    deplace++;

                    if (_boxes[x + _x * (yPlay - 1)].color == _boxes[xPlay + _x * yPlay].color)
                    {
                        // found a lateral color matching with play color
                        foundLateral = true;
                        break;
                    }

                    x--;
                }
                while(x >= 0 && _boxes[x + _x * yPlay].color == colorThrough);

                if (deplace > 0 && (x >= 0) && foundLateral)
                {
                    // phase 1.1: can the dragged box group being dragged ?
                    if (deplace <= leftMax)
                        err = ok;
                }
            }
        }

        return err;
    }

    eError canPlayLeft(int xPlay, int yPlay, int upMax, int downMax, int leftMax, int rightMax, int& deplace)
    {
        eError err = error_cannot_play;
        if (xPlay > 0)
        {
            // phase 1: can a colored box being dragged through a color toward another box with the same color ?
            int x = xPlay - 1;
            eColor colorThrough = _boxes[x + _x * yPlay].color;
            deplace = 0;
            do
            {
                deplace++;
                x--;
            }
            while(x >= 0 && _boxes[x + _x * yPlay].color == colorThrough);

            if (deplace > 0 && (x >= 0) && _boxes[x + _x * yPlay].color == _boxes[xPlay + _x * yPlay].color)
            {
                // phase 1.1: can the dragged box group being dragged ?
                if (deplace <= leftMax)
                    err = ok;
            }

            if (err != ok && deplace > 0 && yPlay < _y - 1)
            {
                // phase 2: can a colored box being dragged through a color toward another LATERAL box with the same color ?
                x = xPlay - 1;
                deplace = 0;
                bool foundLateral = false;
                do
                {
                    deplace++;

                    if (_boxes[x + _x * (yPlay + 1)].color == _boxes[xPlay + _x * yPlay].color)
                    {
                        // found a lateral color matching with play color
                        foundLateral = true;
                        break;
                    }

                    x--;
                }
                while(x >= 0 && _boxes[x + _x * yPlay].color == colorThrough);

                if (deplace > 0 && (x >= 0) && foundLateral)
                {
                    // phase 1.1: can the dragged box group being dragged ?
                    if (deplace <= leftMax)
                        err = ok;
                }
            }

            if (err != ok && deplace > 0 && yPlay > 0)
            {
                // phase 2.1: can a colored box being dragged through a color toward another LATERAL box with the same color ?
                x = xPlay - 1;
                deplace = 0;
                bool foundLateral = false;
                do
                {
                    deplace++;

                    if (_boxes[x + _x * (yPlay - 1)].color == _boxes[xPlay + _x * yPlay].color)
                    {
                        // found a lateral color matching with play color
                        foundLateral = true;
                        break;
                    }

                    x--;
                }
                while(x >= 0 && _boxes[x + _x * yPlay].color == colorThrough);

                if (deplace > 0 && (x >= 0) && foundLateral)
                {
                    // phase 1.1: can the dragged box group being dragged ?
                    if (deplace <= leftMax)
                        err = ok;
                }
            }
        }

        return err;
    }

    eError canPlayRight(int xPlay, int yPlay, int upMax, int downMax, int leftMax, int rightMax, int& deplace)
    {
        eError err = error_cannot_play;
        if (xPlay < (_x - 1))
        {
            // phase 1: can a colored box being dragged through a color toward other boxes with the same color ?
            int x = xPlay + 1;
            eColor colorThrough = _boxes[x + _x * yPlay].color;
            deplace = 0;
            do
            {
                deplace++;
                x++;
            }
            while(x < _x && _boxes[x + _x * yPlay].color == colorThrough);

            if (deplace > 0 && (x < _x) && _boxes[x + _x * yPlay].color == _boxes[xPlay + _x * yPlay].color)
            {
                // phase 1.1: can the dragged box group being dragged ?
                if (deplace <= rightMax)
                    err = ok;
            }
        }
        return err;
    }

    eError canPlayUp(int xPlay, int yPlay, int upMax, int downMax, int leftMax, int rightMax, int& deplace)
    {
        eError err = error_cannot_play;
        if (yPlay > 0)
        {
            // phase 1: can a colored box being dragged through a color toward other boxes with the same color ?
            int y = yPlay - 1;
            eColor colorThrough = _boxes[xPlay + _x * y].color;
            deplace = 0;
            do
            {
                deplace++;
                y--;
            }
            while(y >= 0 && _boxes[xPlay + _x * y].color == colorThrough);

            if (deplace > 0 && (y >= 0) && _boxes[xPlay + _x * y].color == _boxes[xPlay + _x * yPlay].color)
            {
                // phase 2: can the dragged box group being dragged ?
                if (deplace <= upMax)
                    err = ok;
            }
        }
        return err;
    }

    eError canPlayDown(int xPlay, int yPlay, int upMax, int downMax, int leftMax, int rightMax, int& deplace)
    {
        eError err = error_cannot_play;
        if (yPlay < (_y - 1))
        {
            // phase 1: can a colored box being dragged through a color toward other boxes with the same color ?
            int y = yPlay + 1;
            eColor colorThrough = _boxes[xPlay + _x * y].color;
            deplace = 0;
            do
            {
                deplace++;
                y++;
            }
            while(y < _y && _boxes[xPlay + _x * y].color == colorThrough);

            if (deplace > 0 && (y < _y) && _boxes[xPlay + _x * y].color == _boxes[xPlay + _x * yPlay].color)
            {
                // phase 2: can the dragged box group being dragged ?
                if (deplace <= downMax)
                    err = ok;
            }
        }
        return err;
    }
};

Table* generateRandTable(int x, int y, int color_nb)
{
    Table* table = new Table(x, y, color_nb);

    srand((unsigned int)time(0) * 1000);
    for(int index = 0; index < table->getX() * table->getY(); index++)
        table->setColor(index, (eColor)(((float)rand() / RAND_MAX) * table->getColors()));

    return table;
}

Table* generateUTable()
{
    Table* table = new Table(5, 3, 2);
    eColor colors[] = {
        blue,   blue,   blue,   blue,   blue,
        blue,   blue,   red,    red,    blue,
        blue,   red,    blue,   blue,   red
    };

    if (table->setColors(colors, 5 * 3) != ok)
        cout << "generateUTable() failed";

    return table;
}

void inputAndPlay(Table& table)
{
    int x, y;
    string direction_str;
    eDirection direction;
    int deplace;
    eError error;

    // 2. input
    while (true)
    {
        cout << "x > ";
        cin >> x;

        cout << "y > ";
        cin >> y;

        cout << "l,r,u,d > ";
        cin >> direction_str;
        for (int i = 0; i < direction_max; i++)
            if (direction_str == s_directions[i] || direction_str[0] == s_directions[i][0])
                direction = (eDirection)i;

        error = table.play(x, y, direction, deplace);
        if (error == ok)
            cout << "ok, deplace " << deplace << endl;
        else
            cout << "error = " << s_errors[error] << endl;
    }
}

void findAll(Table& table, int x, int y)
{
    int deplace;

    cout << "You can play these (x, y, direction, deplace):" << endl;
    for (int j = 0; j < y; j++)
        for (int i = 0; i < x; i++)
            for (int d = 0; d < (int)direction_max ; d++)
                if (table.play(i, j, (eDirection)d, deplace) == ok)
                    cout << "(" << i << ", " << j << ", " << s_directions[d] << ", " << deplace << ")" << endl;
}

int main(int argc, char* argv[])
{
    // 0. new table with random cases
    //Table& table = *generateRandTable(6, 9, 6);
    Table& table = *generateUTable();

    // 1. numerate
    table.numerate();
    table.print();

    // 2. input
    //inputAndPlay(table);
    findAll(table, 5, 3);

    return 0;
}
