from flask import Flask, render_template
from iot import IoT

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/dados')
def dados():
    print(f'Dados do iot: {IoT().get_dados()}')
    return IoT().get_dados()

if __name__ == '__main__':
    app.run(debug=True)