// Configuration for Christmas Lockbox Puzzles
const CONFIG = {
    // MQTT Broker Details
    MQTT: {
        HOST: "167.172.211.213",
        PORT: 9001, // Websocket port
        CLIENT_ID: "client_" + Math.random().toString(16).substr(2, 8)
    },

    // Topics / Boxes
    // id: used for character mapping in TTS
    // topic: MQTT topic base
    BOXES: [
        { id: "kristine", name: "Kristine's Box", topic: "lockbox/1" },
        { id: "jacob", name: "Jacob's Box", topic: "lockbox/2" },
        { id: "sam", name: "Sam's Box", topic: "lockbox/3" }
    ],
    // Show Assets
    SHOW_SEQUENCE: "show_sequence.json"
};
