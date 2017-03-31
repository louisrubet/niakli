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
    color_red,
    color_green,
    color_yellow,
    color_blue,
    color_majenta,
    color_cyan,
    color_white,
    color_none,
    color_max
};

static string s_colors[color_max] = { BG_RED, BG_GREEN, BG_YELLOW, BG_BLUE, BG_MAJENTA, BG_CYAN, BG_WHITE, BG_BLACK };

class Table
{
public:
    Table(int x, int y, int color_nb):_x(x),_y(y),_color_nb(color_nb),_group_nb(0),
        _cases(new Case[x * y]()),_groups(NULL) { }
    virtual ~Table()
    {
        if (_cases != NULL)
            delete[] _cases;
        if (_groups != NULL)
            delete[] _groups;
    }

    // accessors
    int getX() { return _x; }
    int getY() { return _y; }
    int getColors() { return _color_nb; }
    void setColor(int index, eColor color) { _cases[index].color = color; }

    // 1. numerate and prepare
    void numerate()
    {
        // set same group id to cases with same color
        _group_nb = 0;
        for(int index = 0; index < _x * _y; index++)
            if (_cases[index].group == -1)
                numerate_recurse(index, _group_nb++);

        // get groups properties
        // here assert that all cases have a group between 0 and _group_nb-1
        if (_group_nb > 0)
        {
            _groups = new Group[_group_nb]();
            for(int index = 0; index < _x * _y; index++)
            {
                if (_groups[_cases[index].group].xmin > (index%_x))
                    _groups[_cases[index].group].xmin = (index%_x);
                if (_groups[_cases[index].group].ymin > (index/_x))
                    _groups[_cases[index].group].ymin = (index/_x);
                if (_groups[_cases[index].group].xmax < (index%_x))
                    _groups[_cases[index].group].xmax = (index%_x);
                if (_groups[_cases[index].group].ymax < (index/_x))
                    _groups[_cases[index].group].ymax = (index/_x);
            }
        }
    }

    void print()
    {
        for(int y = 0; y < _y; y++)
        {
            for(int x = 0; x < _x; x++)
            {
                cout << s_colors[_cases[x+_x*y].color]
                    //<< setw(2) << _cases[x+_x*y].group
                    << "  "
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
    struct Case
    {
        Case():color(color_none),group(-1) { }
        eColor color;
        int group;
    };

    struct Group
    {
        Group():xmin(INT_MAX),ymin(INT_MAX),xmax(INT_MIN),ymax(INT_MIN) { }
        int xmin;
        int ymin;
        int xmax;
        int ymax;
    };

    //
    int _x;
    int _y;
    int _color_nb;
    int _group_nb;
    Case* _cases;
    Group* _groups;

    //
    void numerate_recurse(int index, int group)
    {
        eColor color = _cases[index].color;
        _cases[index].group = group;

        if ( ((index%_x) > 0) && (_cases[index - 1].group == -1) && (_cases[index - 1].color == color))
            numerate_recurse(index - 1, group); // left
        if ( ((index%_x) < (_x - 1)) && (_cases[index + 1].group == -1) && (_cases[index + 1].color == color))
            numerate_recurse(index + 1, group); // right
        if ( ((index/_x) > 0) && (_cases[index - _x].group == -1) && (_cases[index - _x].color == color))
            numerate_recurse(index - _x, group); // up
        if ( ((index/_x) < (_y - 1)) && (_cases[index + _x].group == -1) && (_cases[index + _x].color == color))
            numerate_recurse(index + _x, group); // down
    }
};

void generateRandTable(Table& table)
{
    srand((unsigned int)time(0) * 1000);
    for(int index = 0; index < table.getX() * table.getY(); index++)
        table.setColor(index, (eColor)(((float)rand() / RAND_MAX) * table.getColors()));
}

int main(int argc, char* argv[])
{
    Table table(6, 9, 6);

    // 0. new table with random cases
    generateRandTable(table);

    //1. numerate
    table.numerate();
    table.print();

    return 0;
}
