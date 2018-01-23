#include "sensemonstermanager.h"
#include "playernotinsight.h"
#include "noheatededgefound.h"
#include "dumbmonstermanager.h"

SenseMonsterManager::SenseMonsterManager(const Board* board) :
        MonsterManager(board) {
    srand(time(NULL));
}

void SenseMonsterManager::moveMonsters(const Position<>& playerPosition) {
    for(std::pair<const Monster* const, Position<double>> pair : monsters()) {
        Sommet<Case>* monsterVertice = board()->sommet(pair.first->position());
        try {
            monsters()[pair.first] = nextPositionBySight(monsterVertice, playerPosition);
        }
        catch (PlayerNotInSight e) {
            try {
                monsters()[pair.first] = nextPositionByHeat(monsterVertice);
            }
            catch (NoHeatedEdgeFound f) {
                monsters()[pair.first] = DumbMonsterManager::getNextPosition(board(), monsterVertice);
            }
        }
    }
}

Position<> SenseMonsterManager::nextPositionBySight(const Sommet<Case>* monsterVertice, const Position<>& playerPosition) {
    Position<> monsterPosition = monsterVertice->contenu().position();

    if(monsterPosition == playerPosition) {
        return playerPosition;
    }

    Position<> vect = playerPosition - monsterPosition;

    int absX = abs(vect.x);
    int absY = abs(vect.y);
    if(absX != absY && vect.x != 0 && vect.y != 0) {
        throw PlayerNotInSight("Le monstre n'est pas aligné avec le joueur");
    }

    Position<> direction(0, 0);

    if(vect.x != 0) {
        direction.x = vect.x / absX;
    }

    if(vect.y != 0) {
        direction.y = vect.y / absY;
    }

    Position<> next = monsterPosition + direction;
    Liste<Sommet<Case>>* tmp = board()->voisins(monsterVertice);
    Liste<Sommet<Case>>* neighbors = tmp;
    while(neighbors) {
        if(neighbors->value->contenu().position() == next) {
            if(next == playerPosition) {
                Liste<Sommet<Case>>::efface1(tmp);
                return monsterPosition + direction;
            }
            else {
                next = next + direction;
                neighbors = board()->voisins(neighbors->value);
                Liste<Sommet<Case>>::efface1(tmp);
                tmp = neighbors;
            }
        }
        else {
            neighbors = neighbors->next;
        }
    }

    Liste<Sommet<Case>>::efface1(tmp);
    throw PlayerNotInSight("Il n'y a pas de chemin entre le monstre et le joueur");
}

Position<> SenseMonsterManager::nextPositionByHeat(const Sommet<Case>* monsterVertice) {
    Arete<Chemin, Case>* res = nullptr;

    Liste<Arete<Chemin, Case>>* tmp = board()->aretesAdjacentes(monsterVertice);

    for(Liste<Arete<Chemin, Case>>* edges = tmp; edges; edges = edges->next) {
        double chaleur = edges->value->contenu().chaleur();
        if(chaleur > 0 && (!res || chaleur > res->contenu().chaleur())) {
            res = edges->value;
        }
    }

    Liste<Arete<Chemin, Case>>::efface1(tmp);

    if(res == nullptr) {
        throw NoHeatedEdgeFound("Aucune arête chaude trouvée");
    }

    if(monsterVertice == res->debut()) {
        return res->fin()->contenu().position();
    }
    else {
        return res->debut()->contenu().position();
    }
}
