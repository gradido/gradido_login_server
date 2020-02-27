#include "ImportantTests.h"

#include <string>
#include "ServerConfig.h"
#include "Crypto/KeyPair.h"

namespace ImportantTests {

	bool passphraseGenerationAndTransformation()
	{
		auto de_words = &ServerConfig::g_Mnemonic_WordLists[ServerConfig::MNEMONIC_GRADIDO_BOOK_GERMAN_RANDOM_ORDER];
		auto en_words = &ServerConfig::g_Mnemonic_WordLists[ServerConfig::MNEMONIC_BIP0039_SORTED_ORDER];

		std::string passphrase_1_de = "beziffern Anbeginn h�keln Sozialabgaben Rasen flie�en Frau weltweit Urlaub Urwissen Lohn pl�tzlich Gefrierpunkt Derartig Biedermeier getragen denken Realisierung Boden maximal voneinander F�rdern Braten Entlastung";
		std::string passphrase_1_en = "boil banner regret since goat awful crane imitate myth clump rally offer train airport purpose machine helmet ahead alley gesture load scrub river glory";
		std::string passphrase_1_pubkey_hex = "cfce9cfad355ceb8c099a97f55a2bd7aa8d2bd0b86970f7d1e135be9e1da5eb7";

		std::string passphrase_2_de = "dazu Zyklus Danach Auge losfliegen besprechen sto�en ohne heutige Begr�ndung Dogma Erkenntnis genie�en Medaille �ste Google woher Sprache P�dagoge Schweigen rasant Sekunde nahm Nordstern";
		std::string passphrase_2_en = "place oblige gain jar neither note cry riot empty inform egg skate suffer garlic lake ladder liquid focus gorilla subject strong much oyster reduce";
		std::string passphrase_2_pubkey_hex = "3d547825bb53465579b95560981f444105495f2b6a68134fbec28ce518ac7b38";

		KeyPair keys;
		bool errorsOccured = false;
		keys.generateFromPassphrase(passphrase_1_de.data(), de_words);
		if (keys.getPubkeyHex() != passphrase_1_pubkey_hex) {
			printf("1 de incorrect\n");
			errorsOccured = true;
		}
		keys.generateFromPassphrase(passphrase_1_en.data(), en_words);
		if (keys.getPubkeyHex() != passphrase_1_pubkey_hex) {
			printf("1 en incorrect\n");
			errorsOccured = true;
		}

		keys.generateFromPassphrase(passphrase_2_de.data(), de_words);
		if (keys.getPubkeyHex() != passphrase_2_pubkey_hex) {
			printf("2 de incorrect\n");
			errorsOccured = true;
		}
		keys.generateFromPassphrase(passphrase_2_en.data(), en_words);
		if (keys.getPubkeyHex() != passphrase_2_pubkey_hex) {
			printf("2 en incorrect\n");
			errorsOccured = true;
		}

		if (!errorsOccured) return true;
		return false;
	}

}