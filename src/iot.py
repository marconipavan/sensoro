import paho.mqtt.client as mqtt
import threading

class IoT:
    _instance = None

    _broker = 'localhost'
    _port = 1883
    _topic = 'dados/sensoro'

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            print('Iniciando IoT')
            cls._instance._dados_sensoro = ""
            client = mqtt.Client()
            client.on_connect = cls._instance.__on_connect
            client.on_message = cls._instance.__on_message
            client.connect(IoT._broker, IoT._port, 60)
            #client.publish(topic, "Hello from Python!")
            cls._instance.thread = threading.Thread(target=client.loop_forever)
            cls._instance.thread.daemon = True
            cls._instance.thread.start()

        return cls._instance

    def __on_connect(self, client, userdata, flags, rc):
        client.subscribe(IoT._topic)

    def __on_message(self, client, userdata, msg):
        self._dados_sensoro = msg.payload.decode()
    
    def get_dados(self):
        return self._dados_sensoro