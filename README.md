# Pokémon Channel Homebrew Multiboot
This is a homebrew recreation of the Pokémon Channel multiboot.

It doesn't need to communicate with a Gamecube, so it can be used with flashcarts and devices like the mb_adv.

## Changes from Channel Multiboot
The following restrictions have been lifted:
- Needing to be in a Pokémon Center
- Needing to have the National Dex in FireRed/LeafGreen
- Needing to enter the Hall of Fame in Ruby/Sapphire
- Needing to beat Roxanne four times in Emerald
- Needing to activate the Black Flute in FireRed/LeafGreen
- Only being allowed to redeem one Jirachi per save file

## RNG Mechanics
The RNG is seeded based off the time between booting the multiboot and the first button press, down to the microsecond.

## Credits
This project would not have been possible without the following projects:

- [shaonova's EReaderJirachiGenerator](https://discord.com/channels/398682212230365185/440986798559199252/1337842985446146131) - Understanding Channel Jirachi's RNG
- [Gen3-to-Gen-X](https://github.com/Lorenzooone/Pokemon-Gen3-to-Gen-X/tree/main) - Used as a basis for the multiboot and save editing.
    - [PKHeX](https://github.com/kwsch/PKHeX) - Used in Gen3-to-Gen-X to understand legitimacy checks.
    - [PokeFinder](https://github.com/Admiral-Fish/PokeFinder) - Used in Gen3-to-Gen-X to understand RNG and calculations.
    - [pret](https://github.com/pret) - Used in Gen3-to-Gen-X to understand the innermost workings of the games.

## Known Bugs
- Jirachi sometimes isn't registered to the Pokédex

## License
All Pokémon names and names of related resources are © Nintendo/Creatures Inc./GAME FREAK Inc.

Everything else, and the programming code, is governed by the MIT license.

## Building
Docker allows easily building without having to install the dependencies.
- While in the root of the project, run: `docker run --rm -it -v ${PWD}:/home/builder/pokemon_gen3_to_genx lorenzooone/pokemon_gen3_to_genx:gba_builder`
