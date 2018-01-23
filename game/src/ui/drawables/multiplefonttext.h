#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <string>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/String.hpp>

class MultipleFontText : public sf::Drawable, public sf::Transformable {
    public:
        MultipleFontText(const std::wstring& text, const sf::Font& font, unsigned int fontSize);

        /**
         * @brief Ajoute une police de caractère dans le texte
         * @param begin Caractère de début d'intervalle
         * @param end Caractère de fin d'intervalle
         * @param font Police
         */
        void addFont(sf::Uint32 begin, sf::Uint32 end, const sf::Font& font);

        /**
         * @brief Donne les bordures du texte
         * @return Bordures
         */
        const sf::FloatRect& bounds() const;

        inline unsigned int fontSize() const;

        void setColor(const sf::Color& color);
    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void createText() const;
        sf::Font& getFont(sf::Uint32 c) const;

        sf::String _text;
        sf::Color _color;
        mutable sf::Font _defaultFont;
        unsigned int _fontSize;

        mutable std::vector<std::pair<std::pair<int, int>, sf::Font>> _fonts;
        mutable std::vector<sf::Sprite> _glyphs;
        mutable sf::FloatRect _bounds;

        mutable bool _needsUpdate;
};

unsigned int MultipleFontText::fontSize() const {
    return _fontSize;
}
