#include <iostream>
#include <game/jeu.h>
#include <ui/menu/menubutton.h>
#include "mainmenu.h"

MainMenu::MainMenu(sf::RenderWindow* window) :
    Menu(window) {

    addItem(new MenuButton(L"Nouvelle partie", std::bind(&MainMenu::onNewGameSelected, this)));
    addItem(new MenuButton(L"Meilleurs scores", std::bind(&MainMenu::onHighScoreSelected, this)));
    addItem(new MenuButton(L"Crédits", std::bind(&MainMenu::onCreditsSelected, this)));
    addItem(new MenuButton(L"Quitter", std::bind(&MainMenu::onExitSelected, this)));
}

void MainMenu::onNewGameSelected() {
    Jeu *j = new Jeu(4);
}

void MainMenu::onHighScoreSelected() {

}

void MainMenu::onCreditsSelected() {

}

void MainMenu::onExitSelected() {
    _window->close();
}
