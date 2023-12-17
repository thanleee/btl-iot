class User:
    def __init__(self , user_id , taikhoan , matkhau):
        self.user_id = user_id
        self.taikhoan = taikhoan
        self.matkhau = matkhau
    def to_dict(self):
        return {
            'id':self.user_id,
            'taikhoan': self.taikhoan,
            'matkhau': self.matkhau
        }
    def __str__(self) -> str:
        return str(self.user_id) + " "  + self.taikhoan + " " + self.matkhau