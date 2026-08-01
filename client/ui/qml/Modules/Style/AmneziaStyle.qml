pragma Singleton

import QtQuick

QtObject {
    property QtObject color: QtObject {
        readonly property color transparent: 'transparent'
        readonly property color paleGray: '#D7D8DB'
        readonly property color lightGray: '#C1C2C5'
        readonly property color mutedGray: '#878B91'
        readonly property color charcoalGray: '#494B50'
        readonly property color slateGray: '#2C2D30'
        readonly property color onyxBlack: '#1C1D21'
        readonly property color midnightBlack: '#0E0E11'
        readonly property color goldenApricot: goldenApricotString
        readonly property color benefitsPanelBackground: '#1C1C1E'
        readonly property color softViolet: '#A87BE2'
        // Accent family: cold steel blue. Property names are the original amber
        // ones on purpose — renaming them would mean editing 95 QML files, i.e.
        // a permanent merge conflict with upstream for a cosmetic gain. Aliases
        // with honest names are declared at the bottom; prefer those in new code.
        readonly property color burntOrange: '#2A6389'
        readonly property color mutedBrown: '#3E5D75'
        readonly property color richBrown: '#0A3247'
        readonly property color deepBrown: '#06202F'
        readonly property color vibrantRed: '#EB5757'
        readonly property color darkCharcoal: '#1A2027'
        readonly property color pearlGray: '#EAEAEC'

        readonly property color sheerWhite: Qt.rgba(1, 1, 1, 0.12)
        readonly property color translucentWhite: Qt.rgba(1, 1, 1, 0.08)
        readonly property color barelyTranslucentWhite: Qt.rgba(1, 1, 1, 0.05)
        readonly property color translucentMidnightBlack: Qt.rgba(14/255, 14/255, 17/255, 0.8)
        readonly property color softGoldenApricot: Qt.rgba(155/255, 191/255, 222/255, 0.3)
        readonly property color mistyGray: Qt.rgba(215/255, 216/255, 219/255, 0.8)
        readonly property color cloudyGray: Qt.rgba(215/255, 216/255, 219/255, 0.65)
        readonly property color translucentRichBrown: Qt.rgba(10/255, 50/255, 71/255, 0.26)
        readonly property color translucentSlateGray: Qt.rgba(85/255, 86/255, 92/255, 0.13)
        readonly property color translucentOnyxBlack: Qt.rgba(28/255, 29/255, 33/255, 0.13)

        // Main accent. Consumed as a string by TermsAndPrivacyText.qml and
        // BaseHeaderType.qml, which build HTML links, so it has to stay a string.
        readonly property string goldenApricotString: '#9BBFDE'

        // Honest aliases for the accent family. Same values as the legacy names
        // above — use these when writing new QML.
        readonly property string steelBlueString: goldenApricotString
        readonly property color steelBlue: goldenApricotString
        readonly property color softSteelBlue: softGoldenApricot
        readonly property color deepSteel: burntOrange
        readonly property color mutedSteel: mutedBrown
        readonly property color darkSteel: richBrown
        readonly property color deepestSteel: deepBrown
        readonly property color translucentDarkSteel: translucentRichBrown
        readonly property color coolCharcoal: darkCharcoal
    }
}
