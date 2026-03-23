from flask import Flask, request, jsonify
import os
from ultralytics import YOLO
import shutil
from firebase_admin import credentials, initialize_app, firestore
import base64
import glob
from datetime import datetime
import pytz 

app = Flask(__name__)

# --- CẤU HÌNH ---
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
RESULTS_DIR = r"KhoiXuLy\results" # Thư mục lưu ảnh kết quả
MODEL_PATH = os.path.join(BASE_DIR, "best_10k_n.pt")    # model yolo đã train được 

model = YOLO(MODEL_PATH)

cred = credentials.Certificate(os.path.join(BASE_DIR, 'firebase_key.json')) # liên kết firebase
initialize_app(cred)
db = firestore.client()

def image_to_base64(image_path):
    with open(image_path, "rb") as image_file:
        encoded_string = base64.b64encode(image_file.read()).decode('utf-8')
        return f"data:image/jpeg;base64,{encoded_string}"

def get_current_time():
    # Lấy giờ Việt Nam
    tz_VN = pytz.timezone('Asia/Ho_Chi_Minh') 
    return datetime.now(tz_VN).strftime("%d/%m/%Y %H:%M:%S")

@app.route('/process', methods=['POST'])
def process():
    try:
        data = request.get_json()
        patient_id = data.get('id')
        
        if not patient_id:
            return jsonify({"error": "Missing patient ID"}), 400

        # 1. Kiểm tra ảnh đầu vào
        img_input_path = os.path.join(BASE_DIR, f"CT_images/{patient_id}.jpg")
        print(f"--- Đang xử lý: {patient_id} lúc {get_current_time()} ---")
        
        if not os.path.exists(img_input_path):
            return jsonify({"error": "Không tìm thấy ảnh gốc"}), 404

        # 2. Lấy thông tin bệnh nhân (để lấy SĐT)
        patient_ref = db.collection('patients').document(patient_id)
        patient_doc = patient_ref.get()
        
        phone_number = ""
        if patient_doc.exists:
            phone_number = patient_doc.to_dict().get('phone', '')
        else:
            # Nếu chưa có thì tạo mới document bệnh nhân (chưa có lịch sử)
            patient_ref.set({
                'name': 'Unknown', 'phone': '', 'year': ''
            })

        # 3. Chạy Model YOLO
        # Tạo tên folder theo timestamp để tránh ghi đè 
        timestamp_folder = datetime.now().strftime("%Y%m%d_%H%M%S")
        results = model(img_input_path, save=True, project=RESULTS_DIR, name=f"{patient_id}_{timestamp_folder}", exist_ok=True)
        
        boxes = results[0].boxes
        result_label = "positive" if len(boxes) > 0 else "negative"

        # 4. Lấy đường dẫn ảnh kết quả
        save_dir = os.path.join(RESULTS_DIR, f"{patient_id}_{timestamp_folder}")
        output_files = glob.glob(os.path.join(save_dir, "*.jpg"))
        
        if not output_files:
            return jsonify({"error": "Lỗi lưu ảnh kết quả"}), 500
        
        final_result_path = output_files[0]
        base64_image = image_to_base64(final_result_path)
        current_time_str = get_current_time()

        # 5. LƯU LỊCH SỬ (SUB-COLLECTION)
        history_ref = patient_ref.collection('history')
        history_ref.add({
            'result': result_label,
            'image_url': base64_image,
            'timestamp': current_time_str,
            'created_at': firestore.SERVER_TIMESTAMP,
            'note': 'System Auto-detect' # Đánh dấu đây là do hệ thống tự chạy
        })
        
        # 6. Trả về kết quả + SĐT cho ESP32
        return jsonify({
            "id": patient_id,
            "result": result_label,
            "phone": phone_number,  # Gửi số điện thoại về cho ESP32
            "timestamp": current_time_str,
            "message": "Saved to history"
        })
    
    except Exception as e:
        print(f"Server error: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/add_patient', methods=['POST'])
def add_patient():
    try:
        data = request.get_json()
        patient_id = data.get('id')
        patient_ref = db.collection('patients').document(patient_id)
        if patient_ref.get().exists:
            return jsonify({"error": "Exists"}), 400
        
        patient_ref.set({
            'name': 'Bệnh nhân mới', 'phone': '', 'year': ''
        })
        return jsonify({"message": "Created"}), 201
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)