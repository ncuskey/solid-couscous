// Configuration for Christmas Lockbox Puzzles
const CONFIG = {
    // MQTT Broker Details
    MQTT: {
        HOST: "167.172.211.213",
        PORT: 9001, // Websocket port
        CLIENT_ID: "client_" + Math.random().toString(16).substr(2, 8)
    },

    // Topics / Boxes
    BOXES: [
        { name: "Lockbox 1", topic: "lockbox/1" },
        { name: "Lockbox 2", topic: "lockbox/2" }
    ]
};
