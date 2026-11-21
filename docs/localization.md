# Localización (traducciones de un juego)

`Localization` guarda los textos del juego en varios idiomas, como las *String Tables* de
Unity. Cada texto tiene una **clave** (`menu.jugar`) y una traducción por idioma.

## La tabla

Crea `localization.csv` junto a los scripts del proyecto (la carpeta `lua/`). Se carga sola la
primera vez que pides un texto. Es un CSV normal (UTF-8, separado por comas), así que se edita
en Excel, Google Sheets o LibreOffice:

```csv
key,es,en,fr,pt-BR
menu.jugar,Jugar,Play,Jouer,Jogar
menu.salir,Salir,Quit,Quitter,Sair
hud.monedas,"Monedas: {n}","Coins: {n}","Pièces : {n}","Moedas: {n}"
dialogo.hola,"¡Hola, {nombre}!\n¿Listo?","Hi, {nombre}!\nReady?",,
```

- La primera fila es `key` y los códigos de idioma (BCP 47: `es`, `en`, `es-MX`, `pt-BR`, `zh-Hans`...).
- `\n` dentro de una celda es un salto de línea; las celdas con comas van entre comillas.
- Las filas vacías y las que empiezan con `#` se ignoran (sirven de comentarios).
- Una celda vacía usa el idioma por defecto del proyecto, luego el primer idioma de la tabla y,
  si nada, la clave misma: una traducción que falta se nota pero nunca queda en blanco.

Más tablas se cargan con `Localization.load("dialogos.csv")`; se mezclan con las anteriores y
la última gana si una clave se repite.

## Desde Lua

```lua
texto.text = Localization.get("menu.jugar")
monedas.text = Localization.get("hud.monedas", { n = 120 })
saludo.text = Localization.get("dialogo.hola", { nombre = "Ana" })

Localization.setLanguage("en")      -- cambia el idioma del juego
Localization.setLanguage("")        -- vuelve al idioma del dispositivo
print(Localization.getLanguage())   -- "en"
for _, code in ipairs(Localization.getLanguages()) do print(code) end

-- redibuja los textos cuando cambia el idioma
Localization.onLanguageChanged:add("ui", function(code) refrescarTextos() end)
```

## Qué idioma se usa

1. El que el juego fijó con `Localization.setLanguage` (o `TitleArt.setLanguage`: es el mismo).
2. Los idiomas preferidos del dispositivo (`es-CO` encuentra `es`).
3. El idioma por defecto de **Project Settings > Localization**.
4. El primer idioma de la tabla.

## Desde C++

```cpp
#include "util/Localization.h"

std::string s = Localization::get("menu.jugar");
std::string m = Localization::format("hud.monedas", {{"n", "120"}});
Localization::setLanguage("fr");
```
