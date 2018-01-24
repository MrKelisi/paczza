#include <config.h>
#include "jeu.h"
#include <cmath>
#include "dumbmonstermanager.h"
#include "sensemonstermanager.h"
#include "noremaininglife.h"

Jeu::Jeu() :
        _originalPlayerPosition(3,5) {
    int i = 13;
    int j = 8;


    Position<> m1(6,4);
    Position<> m2(6,5);
    Position<> m3(7,4);
    Position<> m4(7,5);

    _originalMonstersPositions.push_back(m1);
    _originalMonstersPositions.push_back(m2);
    _originalMonstersPositions.push_back(m3);
    _originalMonstersPositions.push_back(m4);

    Liste<Position<>>* positionsReservees = nullptr;
    positionsReservees = new Liste<Position<>>(&m1, positionsReservees);
    positionsReservees = new Liste<Position<>>(&m2, positionsReservees);
    positionsReservees = new Liste<Position<>>(&m3, positionsReservees);
    positionsReservees = new Liste<Position<>>(&m4, positionsReservees);
    positionsReservees = new Liste<Position<>>(&_originalPlayerPosition, positionsReservees);

    _plateau = new Board(i, j, positionsReservees);

    _joueur = new Pacman(_originalPlayerPosition, 3);

    _monsterManager = new SenseMonsterManager(_plateau);

    _monstres = nullptr;
    for(Liste<Position<>>* monstres = positionsReservees->next; monstres; monstres = monstres->next) {
        Monster* monster = new Monster(*(monstres->value));
        _monsterManager->addMonster(monster);
        _monstres = new Liste<Monster>(monster, _monstres);
    }

    Liste<Position<>>::efface1(positionsReservees);

    _stopped = true;
    _onPlayerPositionChanged = nullptr;
}

Jeu::~Jeu() {
    delete _plateau;
    delete _joueur;
    delete _monsterManager;
    Liste<Monster>::efface2(_monstres);
}

void Jeu::updateGame(double timeElapsed) {
    updatePlayers(timeElapsed);
}

void Jeu::updatePlayers(double timeElapsed) {
    if(_stopped) {
        return;
    }

    _timeSinceMove += timeElapsed;

    if(_timeSinceMove >= MOVEMENT_TIME) {
        _timeSinceMove = 0;

        for(Liste<Monster>* monsters = _monstres; monsters; monsters = monsters->next) {
            Position<int> newPosition = _monsterManager->newPosition(monsters->value);
            monsters->value->setPosition(newPosition);

            _oldPositions[monsters->value] = newPosition;
        }
        _monsterManager->moveMonsters(_newPlayerPosition);

        _joueur->setPosition(_newPlayerPosition);

        Direction oldDirection = _direction;
        _direction = _newDirection;
        Position<> nextPlayerPosition = getNextPlayerPosition();

        if(nextPlayerPosition == _newPlayerPosition && oldDirection != _direction) {
            _direction = oldDirection;
            _newDirection = oldDirection;
            nextPlayerPosition = getNextPlayerPosition();
        }

        if (_onPlayerPositionChanged) {
            _onPlayerPositionChanged->onPlayerPositionChanged(_oldPositions[_joueur], _newPlayerPosition);
        }

        _oldPositions[_joueur] = _newPlayerPosition;
        _newPlayerPosition = nextPlayerPosition;
    }
    else {
        double movement = timeElapsed / MOVEMENT_TIME;
        for(Liste<Monster>* monsters = _monstres; monsters; monsters = monsters->next) {
            Position<double> vect = _monsterManager->newPosition(monsters->value) - _oldPositions[monsters->value];

            if(vect.x < 0)
                monsters->value->setDirection(LEFT);
            else if(vect.x == 0)
                monsters->value->setDirection(DOWN);
            else if(vect.x > 0)
                monsters->value->setDirection(RIGHT);
            if(vect.y < 0)
                monsters->value->setDirection(UP);

            monsters->value->setPosition(monsters->value->position() + vect * movement);
        }

        _joueur->setPosition(_joueur->position() + (_newPlayerPosition - _oldPositions[_joueur]) * movement);
    }

    for(Liste<Monster>* monsters = _monstres; monsters; monsters = monsters->next) {
        if(std::abs(_joueur->position().x - monsters->value->position().x) < 0.25 &&
           std::abs(_joueur->position().y - monsters->value->position().y) < 0.25) {
            _stopped = true;
        }
    }
}

Position<> Jeu::getNextPlayerPosition() {
    Position<> actuelle = _joueur->position();
    Position<> moveVect;
    switch (_direction) {
        case LEFT:
            moveVect = Position<>(-1, 0);
            break;

        case RIGHT:
            moveVect = Position<>(1, 0);
            break;

        case UP:
            moveVect = Position<>(0, -1);
            break;

        case DOWN:
            moveVect = Position<>(0, 1);
            break;

        case LEFT_UP:
            moveVect = Position<>(-1, -1);
            break;

        case LEFT_DOWN:
            moveVect = Position<>(-1, 1);
            break;

        case RIGHT_UP:
            moveVect = Position<>(1, -1);
            break;

        case RIGHT_DOWN:
            moveVect = Position<>(1, 1);
            break;
    }

    Position<> next = actuelle + moveVect;

    Sommet<Case>* sommetActuel = _plateau->sommet(actuelle);
    Liste<std::pair<Sommet<Case>*, Arete<Chemin, Case>*>>* voisins = _plateau->adjacences(sommetActuel);

    for(Liste<std::pair<Sommet<Case>*, Arete<Chemin, Case>*>>* sommet = voisins; sommet; sommet = sommet->next) {
        try {
            if(sommet->value->first->contenu().position() == next
               && sommet->value->second->contenu().estAccessible()) {

                Arete<Chemin, Case>* arete = sommet->value->second;
                arete->contenu().setChaleur(UINT8_MAX);

                sommet->value->first->contenu().heberge(*(_joueur));

                Liste<std::pair<Sommet<Case>*, Arete<Chemin, Case>*>>::efface2(voisins);
                return next;
            }
        }
        catch(std::exception e) {}
    }

    Liste<std::pair<Sommet<Case>*, Arete<Chemin, Case>*>>::efface2(voisins);
    return actuelle;
}

void Jeu::start() {
    if(!_stopped) {
        return;
    }

    try {
        joueur()->takeLife();

        _timeSinceMove = 0;

        _oldPositions[_joueur] = _originalPlayerPosition;
        _direction = UP;
        _newDirection = UP;
        joueur()->setPosition(_originalPlayerPosition);
        _newPlayerPosition = getNextPlayerPosition();

        int i = 0;
        for(Liste<Monster>* monsters = monstres(); monsters; monsters = monsters->next) {
            monsters->value->setPosition(_originalMonstersPositions[i]);
            _oldPositions[monsters->value] = _originalMonstersPositions[i];
            i++;
        }

        _monsterManager->reset();

        for(Liste<Arete<Chemin, Case>>* aretes = _plateau->aretes(); aretes; aretes = aretes->next) {
            aretes->value->contenu().setChaleur(0);
        }

        _stopped = false;
    }
    catch(NoRemainingLife e) {

    }
}
