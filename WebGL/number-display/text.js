

class FontManager {
    constructor(fontJsonData) {
        this.data = JSON.parse(fontJsonData);
    }
    getCharacterData(c) {
        let id = c.charCodeAt(0);
        // let c = String.fromCharCode(id);
        for (let symbol of this.data.symbols) {
            if (symbol.id === id) {
                symbol.x = symbol.x/this.data.config.textureWidth;
                symbol.yoffset = symbol.yoffset/this.data.config.textureHeight;
                symbol.y = symbol.y/(this.data.config.textureHeight);
                symbol.width = symbol.width/this.data.config.textureWidth;
                symbol.height = symbol.height/this.data.config.textureHeight;
                return symbol;
            }
        }
    }
    parseString(characters) {
        let uniCodeArr = [];
        for (let c of characters) {
            uniCodeArr.push(c.charCodeAt(0));
        }
        return uniCodeArr;
    }
}


const fontJsonData = `{
    "config": {
        "base": 20,
        "bold": 0,
        "charHeight": 25,
        "charSpacing": 0,
        "face": "Noto Sans",
        "italic": 0,
        "lineSpacing": 0,
        "size": 14,
        "smooth": 1,
        "textureFile": "noto_sans_semicondensed_medium_14.PNG",
        "textureHeight": 256,
        "textureWidth": 256
    },
    "kerning": [
    ],
    "symbols": [
        {
            "height": 0,
            "id": 32,
            "width": 0,
            "x": 1,
            "xadvance": 4,
            "xoffset": 0,
            "y": 15,
            "yoffset": 20
        },
        {
            "height": 13,
            "id": 33,
            "width": 3,
            "x": 2,
            "xadvance": 5,
            "xoffset": 1,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 5,
            "id": 34,
            "width": 6,
            "x": 6,
            "xadvance": 8,
            "xoffset": 1,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 35,
            "width": 11,
            "x": 13,
            "xadvance": 11,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 15,
            "id": 36,
            "width": 9,
            "x": 25,
            "xadvance": 10,
            "xoffset": 0,
            "y": 1,
            "yoffset": 6
        },
        {
            "height": 13,
            "id": 37,
            "width": 15,
            "x": 35,
            "xadvance": 15,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 38,
            "width": 13,
            "x": 51,
            "xadvance": 12,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 5,
            "id": 39,
            "width": 3,
            "x": 65,
            "xadvance": 4,
            "xoffset": 1,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 16,
            "id": 40,
            "width": 6,
            "x": 69,
            "xadvance": 6,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 16,
            "id": 41,
            "width": 5,
            "x": 76,
            "xadvance": 6,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 8,
            "id": 42,
            "width": 9,
            "x": 82,
            "xadvance": 9,
            "xoffset": 0,
            "y": 1,
            "yoffset": 6
        },
        {
            "height": 9,
            "id": 43,
            "width": 9,
            "x": 92,
            "xadvance": 10,
            "xoffset": 0,
            "y": 4,
            "yoffset": 9
        },
        {
            "height": 5,
            "id": 44,
            "width": 4,
            "x": 102,
            "xadvance": 5,
            "xoffset": 0,
            "y": 12,
            "yoffset": 17
        },
        {
            "height": 2,
            "id": 45,
            "width": 6,
            "x": 107,
            "xadvance": 6,
            "xoffset": 0,
            "y": 9,
            "yoffset": 14
        },
        {
            "height": 3,
            "id": 46,
            "width": 3,
            "x": 114,
            "xadvance": 5,
            "xoffset": 1,
            "y": 12,
            "yoffset": 17
        },
        {
            "height": 13,
            "id": 47,
            "width": 7,
            "x": 118,
            "xadvance": 7,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 48,
            "width": 9,
            "x": 126,
            "xadvance": 10,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 49,
            "width": 6,
            "x": 136,
            "xadvance": 10,
            "xoffset": 1,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 50,
            "width": 9,
            "x": 143,
            "xadvance": 10,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 51,
            "width": 9,
            "x": 153,
            "xadvance": 10,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 52,
            "width": 10,
            "x": 163,
            "xadvance": 10,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 53,
            "width": 8,
            "x": 174,
            "xadvance": 10,
            "xoffset": 1,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 54,
            "width": 9,
            "x": 183,
            "xadvance": 10,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 55,
            "width": 10,
            "x": 193,
            "xadvance": 10,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 56,
            "width": 10,
            "x": 204,
            "xadvance": 10,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 57,
            "width": 9,
            "x": 215,
            "xadvance": 10,
            "xoffset": 0,
            "y": 2,
            "yoffset": 7
        },
        {
            "height": 10,
            "id": 58,
            "width": 3,
            "x": 225,
            "xadvance": 5,
            "xoffset": 1,
            "y": 5,
            "yoffset": 10
        },
        {
            "height": 13,
            "id": 59,
            "width": 4,
            "x": 229,
            "xadvance": 5,
            "xoffset": 0,
            "y": 5,
            "yoffset": 10
        },
        {
            "height": 9,
            "id": 60,
            "width": 9,
            "x": 234,
            "xadvance": 10,
            "xoffset": 0,
            "y": 4,
            "yoffset": 9
        },
        {
            "height": 7,
            "id": 61,
            "width": 9,
            "x": 244,
            "xadvance": 10,
            "xoffset": 0,
            "y": 5,
            "yoffset": 10
        },
        {
            "height": 9,
            "id": 62,
            "width": 10,
            "x": 1,
            "xadvance": 10,
            "xoffset": 0,
            "y": 21,
            "yoffset": 9
        },
        {
            "height": 13,
            "id": 63,
            "width": 8,
            "x": 12,
            "xadvance": 8,
            "xoffset": 0,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 15,
            "id": 64,
            "width": 15,
            "x": 21,
            "xadvance": 15,
            "xoffset": 0,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 65,
            "width": 11,
            "x": 37,
            "xadvance": 11,
            "xoffset": 0,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 66,
            "width": 10,
            "x": 49,
            "xadvance": 11,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 67,
            "width": 10,
            "x": 60,
            "xadvance": 11,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 68,
            "width": 11,
            "x": 71,
            "xadvance": 12,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 69,
            "width": 8,
            "x": 83,
            "xadvance": 9,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 70,
            "width": 8,
            "x": 92,
            "xadvance": 9,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 71,
            "width": 11,
            "x": 101,
            "xadvance": 12,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 72,
            "width": 10,
            "x": 113,
            "xadvance": 13,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 73,
            "width": 6,
            "x": 124,
            "xadvance": 6,
            "xoffset": 0,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 17,
            "id": 74,
            "width": 6,
            "x": 131,
            "xadvance": 5,
            "xoffset": -2,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 75,
            "width": 10,
            "x": 138,
            "xadvance": 11,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 76,
            "width": 8,
            "x": 149,
            "xadvance": 9,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 77,
            "width": 14,
            "x": 158,
            "xadvance": 16,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 78,
            "width": 11,
            "x": 173,
            "xadvance": 13,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 79,
            "width": 12,
            "x": 185,
            "xadvance": 13,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 80,
            "width": 9,
            "x": 198,
            "xadvance": 10,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 16,
            "id": 81,
            "width": 12,
            "x": 208,
            "xadvance": 13,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 82,
            "width": 10,
            "x": 221,
            "xadvance": 11,
            "xoffset": 1,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 83,
            "width": 9,
            "x": 232,
            "xadvance": 9,
            "xoffset": 0,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 84,
            "width": 10,
            "x": 242,
            "xadvance": 9,
            "xoffset": 0,
            "y": 19,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 85,
            "width": 10,
            "x": 1,
            "xadvance": 12,
            "xoffset": 1,
            "y": 38,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 86,
            "width": 11,
            "x": 12,
            "xadvance": 10,
            "xoffset": 0,
            "y": 38,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 87,
            "width": 17,
            "x": 24,
            "xadvance": 16,
            "xoffset": 0,
            "y": 38,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 88,
            "width": 11,
            "x": 42,
            "xadvance": 10,
            "xoffset": 0,
            "y": 38,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 89,
            "width": 10,
            "x": 54,
            "xadvance": 10,
            "xoffset": 0,
            "y": 38,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 90,
            "width": 9,
            "x": 65,
            "xadvance": 9,
            "xoffset": 0,
            "y": 38,
            "yoffset": 7
        },
        {
            "height": 17,
            "id": 91,
            "width": 5,
            "x": 75,
            "xadvance": 6,
            "xoffset": 1,
            "y": 38,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 92,
            "width": 7,
            "x": 81,
            "xadvance": 7,
            "xoffset": 0,
            "y": 38,
            "yoffset": 7
        },
        {
            "height": 17,
            "id": 93,
            "width": 5,
            "x": 89,
            "xadvance": 6,
            "xoffset": 0,
            "y": 38,
            "yoffset": 7
        },
        {
            "height": 8,
            "id": 94,
            "width": 10,
            "x": 95,
            "xadvance": 10,
            "xoffset": 0,
            "y": 38,
            "yoffset": 7
        },
        {
            "height": 3,
            "id": 95,
            "width": 10,
            "x": 106,
            "xadvance": 8,
            "xoffset": -1,
            "y": 52,
            "yoffset": 21
        },
        {
            "height": 3,
            "id": 96,
            "width": 5,
            "x": 117,
            "xadvance": 6,
            "xoffset": 0,
            "y": 37,
            "yoffset": 6
        },
        {
            "height": 10,
            "id": 97,
            "width": 9,
            "x": 123,
            "xadvance": 10,
            "xoffset": 0,
            "y": 41,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 98,
            "width": 9,
            "x": 133,
            "xadvance": 11,
            "xoffset": 1,
            "y": 37,
            "yoffset": 6
        },
        {
            "height": 10,
            "id": 99,
            "width": 8,
            "x": 143,
            "xadvance": 8,
            "xoffset": 0,
            "y": 41,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 100,
            "width": 10,
            "x": 152,
            "xadvance": 11,
            "xoffset": 0,
            "y": 37,
            "yoffset": 6
        },
        {
            "height": 10,
            "id": 101,
            "width": 9,
            "x": 163,
            "xadvance": 10,
            "xoffset": 0,
            "y": 41,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 102,
            "width": 7,
            "x": 173,
            "xadvance": 6,
            "xoffset": 0,
            "y": 37,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 103,
            "width": 10,
            "x": 181,
            "xadvance": 11,
            "xoffset": 0,
            "y": 41,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 104,
            "width": 9,
            "x": 192,
            "xadvance": 11,
            "xoffset": 1,
            "y": 37,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 105,
            "width": 3,
            "x": 202,
            "xadvance": 5,
            "xoffset": 1,
            "y": 37,
            "yoffset": 6
        },
        {
            "height": 18,
            "id": 106,
            "width": 5,
            "x": 206,
            "xadvance": 5,
            "xoffset": -1,
            "y": 37,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 107,
            "width": 9,
            "x": 212,
            "xadvance": 10,
            "xoffset": 1,
            "y": 37,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 108,
            "width": 3,
            "x": 222,
            "xadvance": 5,
            "xoffset": 1,
            "y": 37,
            "yoffset": 6
        },
        {
            "height": 10,
            "id": 109,
            "width": 14,
            "x": 226,
            "xadvance": 16,
            "xoffset": 1,
            "y": 41,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 110,
            "width": 9,
            "x": 241,
            "xadvance": 11,
            "xoffset": 1,
            "y": 41,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 111,
            "width": 10,
            "x": 1,
            "xadvance": 10,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 112,
            "width": 9,
            "x": 12,
            "xadvance": 11,
            "xoffset": 1,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 113,
            "width": 10,
            "x": 22,
            "xadvance": 11,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 114,
            "width": 6,
            "x": 33,
            "xadvance": 7,
            "xoffset": 1,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 115,
            "width": 8,
            "x": 40,
            "xadvance": 8,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 12,
            "id": 116,
            "width": 7,
            "x": 49,
            "xadvance": 6,
            "xoffset": 0,
            "y": 58,
            "yoffset": 8
        },
        {
            "height": 10,
            "id": 117,
            "width": 9,
            "x": 57,
            "xadvance": 11,
            "xoffset": 1,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 118,
            "width": 9,
            "x": 67,
            "xadvance": 9,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 119,
            "width": 14,
            "x": 77,
            "xadvance": 14,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 120,
            "width": 9,
            "x": 92,
            "xadvance": 9,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 121,
            "width": 9,
            "x": 102,
            "xadvance": 9,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 122,
            "width": 8,
            "x": 112,
            "xadvance": 8,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 17,
            "id": 123,
            "width": 7,
            "x": 121,
            "xadvance": 7,
            "xoffset": 0,
            "y": 57,
            "yoffset": 7
        },
        {
            "height": 18,
            "id": 124,
            "width": 3,
            "x": 129,
            "xadvance": 9,
            "xoffset": 3,
            "y": 56,
            "yoffset": 6
        },
        {
            "height": 17,
            "id": 125,
            "width": 7,
            "x": 133,
            "xadvance": 7,
            "xoffset": 0,
            "y": 57,
            "yoffset": 7
        },
        {
            "height": 4,
            "id": 126,
            "width": 10,
            "x": 141,
            "xadvance": 10,
            "xoffset": 0,
            "y": 62,
            "yoffset": 12
        },
        {
            "height": 13,
            "id": 880,
            "width": 7,
            "x": 152,
            "xadvance": 8,
            "xoffset": 1,
            "y": 57,
            "yoffset": 7
        },
        {
            "height": 10,
            "id": 881,
            "width": 6,
            "x": 160,
            "xadvance": 7,
            "xoffset": 1,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 13,
            "id": 882,
            "width": 11,
            "x": 167,
            "xadvance": 11,
            "xoffset": 0,
            "y": 57,
            "yoffset": 7
        },
        {
            "height": 14,
            "id": 883,
            "width": 10,
            "x": 179,
            "xadvance": 10,
            "xoffset": 0,
            "y": 56,
            "yoffset": 6
        },
        {
            "height": 3,
            "id": 884,
            "width": 4,
            "x": 190,
            "xadvance": 4,
            "xoffset": 0,
            "y": 57,
            "yoffset": 7
        },
        {
            "height": 3,
            "id": 885,
            "width": 4,
            "x": 195,
            "xadvance": 4,
            "xoffset": 0,
            "y": 71,
            "yoffset": 21
        },
        {
            "height": 13,
            "id": 886,
            "width": 11,
            "x": 200,
            "xadvance": 13,
            "xoffset": 1,
            "y": 57,
            "yoffset": 7
        },
        {
            "height": 10,
            "id": 887,
            "width": 9,
            "x": 212,
            "xadvance": 11,
            "xoffset": 1,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 4,
            "id": 890,
            "width": 4,
            "x": 222,
            "xadvance": 11,
            "xoffset": 4,
            "y": 71,
            "yoffset": 21
        },
        {
            "height": 10,
            "id": 891,
            "width": 8,
            "x": 227,
            "xadvance": 8,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 892,
            "width": 8,
            "x": 236,
            "xadvance": 8,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 893,
            "width": 8,
            "x": 245,
            "xadvance": 8,
            "xoffset": 0,
            "y": 60,
            "yoffset": 10
        },
        {
            "height": 13,
            "id": 894,
            "width": 4,
            "x": 1,
            "xadvance": 5,
            "xoffset": 0,
            "y": 81,
            "yoffset": 10
        },
        {
            "height": 17,
            "id": 895,
            "width": 6,
            "x": 6,
            "xadvance": 5,
            "xoffset": -2,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 4,
            "id": 900,
            "width": 4,
            "x": 13,
            "xadvance": 10,
            "xoffset": 4,
            "y": 76,
            "yoffset": 5
        },
        {
            "height": 4,
            "id": 901,
            "width": 7,
            "x": 18,
            "xadvance": 10,
            "xoffset": 2,
            "y": 76,
            "yoffset": 5
        },
        {
            "height": 14,
            "id": 902,
            "width": 12,
            "x": 26,
            "xadvance": 11,
            "xoffset": 0,
            "y": 77,
            "yoffset": 6
        },
        {
            "height": 3,
            "id": 903,
            "width": 3,
            "x": 39,
            "xadvance": 5,
            "xoffset": 1,
            "y": 81,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 904,
            "width": 12,
            "x": 43,
            "xadvance": 11,
            "xoffset": -1,
            "y": 77,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 905,
            "width": 14,
            "x": 56,
            "xadvance": 14,
            "xoffset": -1,
            "y": 77,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 906,
            "width": 9,
            "x": 71,
            "xadvance": 9,
            "xoffset": -1,
            "y": 77,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 908,
            "width": 14,
            "x": 81,
            "xadvance": 14,
            "xoffset": -1,
            "y": 77,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 910,
            "width": 14,
            "x": 96,
            "xadvance": 13,
            "xoffset": -1,
            "y": 77,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 911,
            "width": 15,
            "x": 111,
            "xadvance": 14,
            "xoffset": -1,
            "y": 77,
            "yoffset": 6
        },
        {
            "height": 15,
            "id": 912,
            "width": 7,
            "x": 127,
            "xadvance": 6,
            "xoffset": -1,
            "y": 76,
            "yoffset": 5
        },
        {
            "height": 13,
            "id": 913,
            "width": 11,
            "x": 135,
            "xadvance": 11,
            "xoffset": 0,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 914,
            "width": 10,
            "x": 147,
            "xadvance": 11,
            "xoffset": 1,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 915,
            "width": 8,
            "x": 158,
            "xadvance": 9,
            "xoffset": 1,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 916,
            "width": 11,
            "x": 167,
            "xadvance": 10,
            "xoffset": 0,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 917,
            "width": 8,
            "x": 179,
            "xadvance": 9,
            "xoffset": 1,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 918,
            "width": 9,
            "x": 188,
            "xadvance": 9,
            "xoffset": 0,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 919,
            "width": 10,
            "x": 198,
            "xadvance": 13,
            "xoffset": 1,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 920,
            "width": 12,
            "x": 209,
            "xadvance": 13,
            "xoffset": 1,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 921,
            "width": 6,
            "x": 222,
            "xadvance": 6,
            "xoffset": 0,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 922,
            "width": 10,
            "x": 229,
            "xadvance": 11,
            "xoffset": 1,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 923,
            "width": 11,
            "x": 240,
            "xadvance": 11,
            "xoffset": 0,
            "y": 78,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 924,
            "width": 14,
            "x": 1,
            "xadvance": 16,
            "xoffset": 1,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 925,
            "width": 11,
            "x": 16,
            "xadvance": 13,
            "xoffset": 1,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 926,
            "width": 9,
            "x": 28,
            "xadvance": 9,
            "xoffset": 0,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 927,
            "width": 12,
            "x": 38,
            "xadvance": 13,
            "xoffset": 1,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 928,
            "width": 10,
            "x": 51,
            "xadvance": 12,
            "xoffset": 1,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 929,
            "width": 9,
            "x": 62,
            "xadvance": 10,
            "xoffset": 1,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 931,
            "width": 10,
            "x": 72,
            "xadvance": 10,
            "xoffset": 0,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 932,
            "width": 10,
            "x": 83,
            "xadvance": 9,
            "xoffset": 0,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 933,
            "width": 10,
            "x": 94,
            "xadvance": 10,
            "xoffset": 0,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 934,
            "width": 14,
            "x": 105,
            "xadvance": 14,
            "xoffset": 0,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 935,
            "width": 11,
            "x": 120,
            "xadvance": 10,
            "xoffset": 0,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 936,
            "width": 13,
            "x": 132,
            "xadvance": 14,
            "xoffset": 1,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 937,
            "width": 13,
            "x": 146,
            "xadvance": 13,
            "xoffset": 0,
            "y": 99,
            "yoffset": 7
        },
        {
            "height": 16,
            "id": 938,
            "width": 6,
            "x": 160,
            "xadvance": 6,
            "xoffset": 0,
            "y": 96,
            "yoffset": 4
        },
        {
            "height": 16,
            "id": 939,
            "width": 10,
            "x": 167,
            "xadvance": 10,
            "xoffset": 0,
            "y": 96,
            "yoffset": 4
        },
        {
            "height": 15,
            "id": 940,
            "width": 11,
            "x": 178,
            "xadvance": 11,
            "xoffset": 0,
            "y": 97,
            "yoffset": 5
        },
        {
            "height": 15,
            "id": 941,
            "width": 8,
            "x": 190,
            "xadvance": 8,
            "xoffset": 0,
            "y": 97,
            "yoffset": 5
        },
        {
            "height": 19,
            "id": 942,
            "width": 9,
            "x": 199,
            "xadvance": 11,
            "xoffset": 1,
            "y": 97,
            "yoffset": 5
        },
        {
            "height": 15,
            "id": 943,
            "width": 5,
            "x": 209,
            "xadvance": 6,
            "xoffset": 1,
            "y": 97,
            "yoffset": 5
        },
        {
            "height": 15,
            "id": 944,
            "width": 9,
            "x": 215,
            "xadvance": 11,
            "xoffset": 1,
            "y": 97,
            "yoffset": 5
        },
        {
            "height": 10,
            "id": 945,
            "width": 11,
            "x": 225,
            "xadvance": 11,
            "xoffset": 0,
            "y": 102,
            "yoffset": 10
        },
        {
            "height": 18,
            "id": 946,
            "width": 10,
            "x": 237,
            "xadvance": 11,
            "xoffset": 1,
            "y": 98,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 947,
            "width": 9,
            "x": 1,
            "xadvance": 9,
            "xoffset": 0,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 948,
            "width": 10,
            "x": 11,
            "xadvance": 10,
            "xoffset": 0,
            "y": 117,
            "yoffset": 6
        },
        {
            "height": 10,
            "id": 949,
            "width": 8,
            "x": 22,
            "xadvance": 8,
            "xoffset": 0,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 18,
            "id": 950,
            "width": 8,
            "x": 31,
            "xadvance": 8,
            "xoffset": 0,
            "y": 117,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 951,
            "width": 9,
            "x": 40,
            "xadvance": 11,
            "xoffset": 1,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 952,
            "width": 10,
            "x": 50,
            "xadvance": 10,
            "xoffset": 0,
            "y": 117,
            "yoffset": 6
        },
        {
            "height": 10,
            "id": 953,
            "width": 5,
            "x": 61,
            "xadvance": 6,
            "xoffset": 1,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 954,
            "width": 9,
            "x": 67,
            "xadvance": 10,
            "xoffset": 1,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 955,
            "width": 10,
            "x": 77,
            "xadvance": 10,
            "xoffset": 0,
            "y": 117,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 956,
            "width": 9,
            "x": 88,
            "xadvance": 11,
            "xoffset": 1,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 957,
            "width": 9,
            "x": 98,
            "xadvance": 9,
            "xoffset": 0,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 18,
            "id": 958,
            "width": 8,
            "x": 108,
            "xadvance": 8,
            "xoffset": 0,
            "y": 117,
            "yoffset": 6
        },
        {
            "height": 10,
            "id": 959,
            "width": 10,
            "x": 117,
            "xadvance": 10,
            "xoffset": 0,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 960,
            "width": 12,
            "x": 128,
            "xadvance": 12,
            "xoffset": 0,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 961,
            "width": 9,
            "x": 141,
            "xadvance": 10,
            "xoffset": 1,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 962,
            "width": 8,
            "x": 151,
            "xadvance": 8,
            "xoffset": 0,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 963,
            "width": 11,
            "x": 160,
            "xadvance": 11,
            "xoffset": 0,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 964,
            "width": 8,
            "x": 172,
            "xadvance": 8,
            "xoffset": 0,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 965,
            "width": 9,
            "x": 181,
            "xadvance": 11,
            "xoffset": 1,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 966,
            "width": 13,
            "x": 191,
            "xadvance": 13,
            "xoffset": 0,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 967,
            "width": 11,
            "x": 205,
            "xadvance": 9,
            "xoffset": -1,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 18,
            "id": 968,
            "width": 12,
            "x": 217,
            "xadvance": 13,
            "xoffset": 1,
            "y": 117,
            "yoffset": 6
        },
        {
            "height": 10,
            "id": 969,
            "width": 13,
            "x": 230,
            "xadvance": 14,
            "xoffset": 0,
            "y": 121,
            "yoffset": 10
        },
        {
            "height": 13,
            "id": 970,
            "width": 7,
            "x": 244,
            "xadvance": 6,
            "xoffset": -1,
            "y": 118,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 971,
            "width": 9,
            "x": 1,
            "xadvance": 11,
            "xoffset": 1,
            "y": 140,
            "yoffset": 7
        },
        {
            "height": 15,
            "id": 972,
            "width": 10,
            "x": 11,
            "xadvance": 10,
            "xoffset": 0,
            "y": 138,
            "yoffset": 5
        },
        {
            "height": 15,
            "id": 973,
            "width": 9,
            "x": 22,
            "xadvance": 11,
            "xoffset": 1,
            "y": 138,
            "yoffset": 5
        },
        {
            "height": 15,
            "id": 974,
            "width": 13,
            "x": 32,
            "xadvance": 14,
            "xoffset": 0,
            "y": 138,
            "yoffset": 5
        },
        {
            "height": 17,
            "id": 975,
            "width": 10,
            "x": 46,
            "xadvance": 11,
            "xoffset": 1,
            "y": 140,
            "yoffset": 7
        },
        {
            "height": 14,
            "id": 976,
            "width": 10,
            "x": 57,
            "xadvance": 10,
            "xoffset": 0,
            "y": 139,
            "yoffset": 6
        },
        {
            "height": 14,
            "id": 977,
            "width": 12,
            "x": 68,
            "xadvance": 11,
            "xoffset": 0,
            "y": 139,
            "yoffset": 6
        },
        {
            "height": 13,
            "id": 978,
            "width": 10,
            "x": 81,
            "xadvance": 10,
            "xoffset": 0,
            "y": 140,
            "yoffset": 7
        },
        {
            "height": 14,
            "id": 979,
            "width": 14,
            "x": 92,
            "xadvance": 12,
            "xoffset": -1,
            "y": 139,
            "yoffset": 6
        },
        {
            "height": 17,
            "id": 980,
            "width": 10,
            "x": 107,
            "xadvance": 10,
            "xoffset": 0,
            "y": 136,
            "yoffset": 3
        },
        {
            "height": 18,
            "id": 981,
            "width": 12,
            "x": 118,
            "xadvance": 13,
            "xoffset": 0,
            "y": 139,
            "yoffset": 6
        },
        {
            "height": 10,
            "id": 982,
            "width": 15,
            "x": 131,
            "xadvance": 15,
            "xoffset": 0,
            "y": 143,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 983,
            "width": 12,
            "x": 147,
            "xadvance": 11,
            "xoffset": -1,
            "y": 143,
            "yoffset": 10
        },
        {
            "height": 13,
            "id": 984,
            "width": 11,
            "x": 160,
            "xadvance": 13,
            "xoffset": 1,
            "y": 140,
            "yoffset": 7
        },
        {
            "height": 14,
            "id": 985,
            "width": 10,
            "x": 172,
            "xadvance": 10,
            "xoffset": 0,
            "y": 143,
            "yoffset": 10
        },
        {
            "height": 13,
            "id": 986,
            "width": 10,
            "x": 183,
            "xadvance": 10,
            "xoffset": 1,
            "y": 140,
            "yoffset": 7
        },
        {
            "height": 15,
            "id": 987,
            "width": 8,
            "x": 194,
            "xadvance": 8,
            "xoffset": 0,
            "y": 143,
            "yoffset": 10
        },
        {
            "height": 13,
            "id": 988,
            "width": 8,
            "x": 203,
            "xadvance": 9,
            "xoffset": 1,
            "y": 140,
            "yoffset": 7
        },
        {
            "height": 14,
            "id": 989,
            "width": 7,
            "x": 212,
            "xadvance": 8,
            "xoffset": 1,
            "y": 143,
            "yoffset": 10
        },
        {
            "height": 13,
            "id": 990,
            "width": 8,
            "x": 220,
            "xadvance": 8,
            "xoffset": 0,
            "y": 140,
            "yoffset": 7
        },
        {
            "height": 18,
            "id": 991,
            "width": 9,
            "x": 229,
            "xadvance": 9,
            "xoffset": 0,
            "y": 139,
            "yoffset": 6
        },
        {
            "height": 13,
            "id": 992,
            "width": 10,
            "x": 239,
            "xadvance": 10,
            "xoffset": -1,
            "y": 140,
            "yoffset": 7
        },
        {
            "height": 19,
            "id": 993,
            "width": 10,
            "x": 1,
            "xadvance": 9,
            "xoffset": -2,
            "y": 159,
            "yoffset": 5
        },
        {
            "height": 10,
            "id": 1008,
            "width": 12,
            "x": 12,
            "xadvance": 11,
            "xoffset": -1,
            "y": 164,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 1009,
            "width": 10,
            "x": 25,
            "xadvance": 10,
            "xoffset": 0,
            "y": 164,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 1010,
            "width": 8,
            "x": 36,
            "xadvance": 8,
            "xoffset": 0,
            "y": 164,
            "yoffset": 10
        },
        {
            "height": 18,
            "id": 1011,
            "width": 5,
            "x": 45,
            "xadvance": 5,
            "xoffset": -1,
            "y": 160,
            "yoffset": 6
        },
        {
            "height": 13,
            "id": 1012,
            "width": 12,
            "x": 51,
            "xadvance": 13,
            "xoffset": 1,
            "y": 161,
            "yoffset": 7
        },
        {
            "height": 10,
            "id": 1013,
            "width": 8,
            "x": 64,
            "xadvance": 9,
            "xoffset": 0,
            "y": 164,
            "yoffset": 10
        },
        {
            "height": 10,
            "id": 1014,
            "width": 8,
            "x": 73,
            "xadvance": 9,
            "xoffset": 0,
            "y": 164,
            "yoffset": 10
        },
        {
            "height": 13,
            "id": 1015,
            "width": 9,
            "x": 82,
            "xadvance": 10,
            "xoffset": 1,
            "y": 161,
            "yoffset": 7
        },
        {
            "height": 18,
            "id": 1016,
            "width": 9,
            "x": 92,
            "xadvance": 11,
            "xoffset": 1,
            "y": 160,
            "yoffset": 6
        },
        {
            "height": 13,
            "id": 1017,
            "width": 10,
            "x": 102,
            "xadvance": 11,
            "xoffset": 1,
            "y": 161,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 1018,
            "width": 13,
            "x": 113,
            "xadvance": 16,
            "xoffset": 1,
            "y": 161,
            "yoffset": 7
        },
        {
            "height": 14,
            "id": 1019,
            "width": 11,
            "x": 127,
            "xadvance": 13,
            "xoffset": 1,
            "y": 164,
            "yoffset": 10
        },
        {
            "height": 14,
            "id": 1020,
            "width": 10,
            "x": 139,
            "xadvance": 10,
            "xoffset": 0,
            "y": 164,
            "yoffset": 10
        },
        {
            "height": 13,
            "id": 1021,
            "width": 10,
            "x": 150,
            "xadvance": 11,
            "xoffset": 0,
            "y": 161,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 1022,
            "width": 10,
            "x": 161,
            "xadvance": 11,
            "xoffset": 1,
            "y": 161,
            "yoffset": 7
        },
        {
            "height": 13,
            "id": 1023,
            "width": 10,
            "x": 172,
            "xadvance": 11,
            "xoffset": 0,
            "y": 161,
            "yoffset": 7
        }
    ]
}`
