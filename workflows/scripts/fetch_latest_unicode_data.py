import os
import sys
import urllib.request
from typing import List

sys.path.append(os.path.abspath(f"{os.path.dirname(__file__)}/.."))

import lib.config as config
from lib.log import lcheck_passed, lprint

ucd_remote_url: str = "https://www.unicode.org/Public/UCD/latest"
ucd_local_dir: str = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "../../sys.Text/data_unicode")
)

ucd_files: List[str] = [
    "ReadMe.txt",
    "charts/RSIndex.txt",
    "charts/Readme.txt",
    "charts/fr/Readme.txt",
    "emoji/ReadMe.txt",
    "emoji/emoji-sequences.txt",
    "emoji/emoji-test.txt",
    "emoji/emoji-zwj-sequences.txt",
    "idna/Idna2008.txt",
    "idna/IdnaMappingTable.txt",
    "idna/IdnaTestV2.txt",
    "idna/ReadMe.txt",
    "linkification/LinkBracket.txt",
    "linkification/LinkDetectionTest.txt",
    "linkification/LinkEmail.txt",
    "linkification/LinkFormattingTest.txt",
    "linkification/LinkTerm.txt",
    "linkification/ReadMe.txt",
    "security/IdentifierStatus.txt",
    "security/IdentifierType.txt",
    "security/ReadMe.txt",
    "security/confusables.txt",
    "security/confusablesSummary.txt",
    "security/intentional.txt",
    "uca/ReadMe.txt",
    "uca/allkeys.txt",
    "uca/ctt.txt",
    "uca/decomps.txt",
    "ucd/ArabicShaping.txt",
    "ucd/BidiBrackets.txt",
    "ucd/BidiCharacterTest.txt",
    "ucd/BidiMirroring.txt",
    "ucd/BidiTest.txt",
    "ucd/Blocks.txt",
    "ucd/CJKRadicals.txt",
    "ucd/CaseFolding.txt",
    "ucd/CompositionExclusions.txt",
    "ucd/DerivedAge.txt",
    "ucd/DerivedCoreProperties.txt",
    "ucd/DerivedNormalizationProps.txt",
    "ucd/DoNotEmit.txt",
    "ucd/EastAsianWidth.txt",
    "ucd/EmojiSources.txt",
    "ucd/EquivalentUnifiedIdeograph.txt",
    "ucd/HangulSyllableType.txt",
    "ucd/Index.txt",
    "ucd/IndicPositionalCategory.txt",
    "ucd/IndicSyllabicCategory.txt",
    "ucd/Jamo.txt",
    "ucd/LineBreak.txt",
    "ucd/NameAliases.txt",
    "ucd/NamedSequences.txt",
    "ucd/NamedSequencesProv.txt",
    "ucd/NamesList.txt",
    "ucd/NormalizationCorrections.txt",
    "ucd/NormalizationTest.txt",
    "ucd/NushuSources.txt",
    "ucd/PropList.txt",
    "ucd/PropertyAliases.txt",
    "ucd/PropertyValueAliases.txt",
    "ucd/ReadMe.txt",
    "ucd/ScriptExtensions.txt",
    "ucd/Scripts.txt",
    "ucd/SpecialCasing.txt",
    "ucd/StandardizedVariants.txt",
    "ucd/TangutSources.txt",
    "ucd/USourceData.txt",
    "ucd/UnicodeData.txt",
    "ucd/Unikemet.txt",
    "ucd/VerticalOrientation.txt",
    "ucd/auxiliary/GraphemeBreakProperty.txt",
    "ucd/auxiliary/GraphemeBreakTest.txt",
    "ucd/auxiliary/LineBreakTest.txt",
    "ucd/auxiliary/SentenceBreakProperty.txt",
    "ucd/auxiliary/SentenceBreakTest.txt",
    "ucd/auxiliary/WordBreakProperty.txt",
    "ucd/auxiliary/WordBreakTest.txt",
    "ucd/emoji/ReadMe.txt",
    "ucd/emoji/emoji-data.txt",
    "ucd/emoji/emoji-variation-sequences.txt",
    "ucd/extracted/DerivedBidiClass.txt",
    "ucd/extracted/DerivedBinaryProperties.txt",
    "ucd/extracted/DerivedCombiningClass.txt",
    "ucd/extracted/DerivedDecompositionType.txt",
    "ucd/extracted/DerivedEastAsianWidth.txt",
    "ucd/extracted/DerivedGeneralCategory.txt",
    "ucd/extracted/DerivedJoiningGroup.txt",
    "ucd/extracted/DerivedJoiningType.txt",
    "ucd/extracted/DerivedLineBreak.txt",
    "ucd/extracted/DerivedName.txt",
    "ucd/extracted/DerivedNumericType.txt",
    "ucd/extracted/DerivedNumericValues.txt",
]


def main(argv: List[str]) -> None:
    config.check_name = "scripts:fetch_latest_unicode_data"

    opener = urllib.request.build_opener()
    opener.addheaders = [
        (
            # Pretend we're a real browser so `unicode.org` doesn't block us.
            "User-Agent",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3",
        )
    ]
    urllib.request.install_opener(opener)

    for ucd_file in ucd_files:
        os.makedirs(os.path.dirname(f"{ucd_local_dir}/{ucd_file}"), exist_ok=True)

        lprint(f"Downloading {ucd_file}...")
        urllib.request.urlretrieve(
            f"{ucd_remote_url}/{ucd_file}",
            f"{ucd_local_dir}/{ucd_file}",
        )
        lprint("Done.")

    lcheck_passed()


if __name__ == "__main__":
    main(sys.argv)
