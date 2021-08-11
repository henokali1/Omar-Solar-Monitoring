from flask import Flask, render_template, send_from_directory, Response, request
import pickle as pickle
from time import time

app = Flask(__name__)
app.config["SECRET_KEY"] = "secret!"


def read_db():
	with open('db.pickle', 'rb') as handle:
		val = pickle.load(handle)
	return val

def write_to_db(val):
	with open('db.pickle', 'wb') as handle:
		pickle.dump(val, handle, protocol=pickle.HIGHEST_PROTOCOL)


@app.route("/")
def dashboard():
	return render_template("dashboard.html")

@app.route('/sensor_data')
def sensor_data():
	sen_data = read_db()
	ts = int(time())
	sen_data['cc'] = request.args.get('cc', default = 0.0, type = float)
	sen_data['lc'] = request.args.get('lc', default = 0.0, type = float)
	sen_data['cv'] = request.args.get('cv', default = 0.0, type = float)
	sen_data['bp'] = request.args.get('bp', default = 0.0, type = float)
	sen_data['bv'] = request.args.get('bv', default = 0.0, type = float)
	sen_data['pw'] = request.args.get('pw', default = 0.0, type = float)
	sen_data['ts'] = ts
	# sen_data={'cc':cc, 'lc': lc, 'bp':bp, 'cv': cv, 'bv': bv, 'pw': pw, 'ts': ts}
	write_to_db(sen_data)
	return sen_data

@app.route('/save_rasp_ip')
def save_rasp_ip():
	ipts = int(time())
	ip = request.args.get('ip', default = '', type = str)
	od = read_db()
	od['ip']=ip
	od['ipts']=ipts
	write_to_db(od)
	return {'ip': ip, 'ipts': ipts}


@app.route('/lattest_data')
def lattest_data():
	d = read_db()
	print(d)
	ots = d['ts']
	cts = int(time())
	t_diff = cts-ots
	ipts = d['ipts']
	ip_diff = cts-ipts
	data_exp = t_diff > 10
	ip_exp = ip_diff > 60
	d['t_diff'] = t_diff
	d['data_exp'] = data_exp
	d['ip_exp']=ip_exp
	return d


if __name__=="__main__":
	# app.run(host=args.host,port=args.port)
	app.run(host='0.0.0.0', port=8000, debug=True)
