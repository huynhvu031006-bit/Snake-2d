# Snake 2D

C++17 + SFML 3.1, Windows x64 / Visual Studio (toolset v145).

## Chay game

Mo `source snake/source snake.vcxproj`, chon Debug hoac Release / x64, build va chay (Ctrl+F5).
Build tu dong copy DLL SFML vao thu muc EXE. Font: Arial cua Windows.

- WASD / mui ten: doi huong; khong duoc quay nguoc 180 do.
- P / Space: tam dung / tiep tuc. Mat focus tu dong tam dung.
- Enter / R: choi lai sau khi thua hoac thang.
- Esc / P / Space: tam dung / tiep tuc. Bam X tren thanh tieu de de thoat.
- Moi moi +10 diem, tang 1 o/giay moi 30 diem, tu 6 toi toi da 18 o/giay.
- Cham tuong hoac than thi thua; lap day ban co thi thang.
- 20 luot gan nhat (ke ca 0 diem), moi nhat truoc, duoc luu khi ket thuc van, tai `%LOCALAPPDATA%\Snake2D\scores.txt`.
  Neu khong co LOCALAPPDATA, dung thu muc EXE. Khong luu van dang choi khi thoat.

## Cau truc

- `source snake/Game.hpp`: logic C++17 doc lap SFML; luoi, input huong, moi, va cham, diem, toc do, trang thai.
- `source snake/Scores.hpp`: doc, sap xep, gioi han 20 luot gan nhat, luu qua file tam va thay the file cu.
- `source snake/source snake.cpp`: SFML input/render, fixed timestep va bo kiem thu `--test`.
- `SFML/`: headers, libraries va DLL da co san.

Game loop dung accumulator voi buoc co dinh 1/120 giay; render rieng toi da 120 FPS.
Nhip di chuyen la 6-18 o/giay, duoc tinh trong fixed update. Gioi han frame delta 0.25 giay de tranh catch-up qua dai.
Moi tick chi nhan mot lan re hop le de tranh quay nguoc qua nhieu su kien ban phim.

## Kiem thu

Sau khi build Debug, chay PowerShell tu thu muc nay:

```powershell
& '.\x64\Debug\source snake.exe' '--test'
```

Khong mo cua so khi chay test. Exit code 0 la thanh cong; loi tra ve 1.
Kiem thu di chuyen, cam quay nguoc, gioi han input, pause, reset, cham tuong,
an moi/tang chieu dai/diem, moi khong trung than, cham than, di vao duoi vua roi,
thang khi day ban, gioi han toc do, loc file diem hong, luu/doc, thu tu gan day va gioi han 20 luot.
Test dung seed co dinh va file tam rieng, khong thay doi bang diem nguoi choi.



Ngon ngu: bam English / Tieng Viet o goc tren ben phai, hoac phim L de doi. Mac dinh Tieng Viet; lua chon ap dung trong phien choi.

Man hinh cho: bam CHOI NGAY / PLAY NOW hoac Enter moi bat dau. Co the chon ngon ngu truoc khi choi. Bang diem hien 20 luot gan nhat theo thu tu moi den cu; khong sap xep theo diem. Diem cu tu ban top 10 van duoc giu, nhung khong khoi phuc duoc thoi gian choi cu.


Dieu khien bo sung: nut Toc do + / phim B tang 1 o/giay, toi da 18; van tu tang sau moi 3 moi. Nut Tam dung / Tiep tuc hoac P / Space. Nut XOA BANG XEP HANG xoa ngay lich su da luu; van dang choi se duoc ghi lai khi ket thuc.

Cua so cho phep keo gian va phong to bang nut Maximize; giao dien giu ti le 1040:760, co vien khi ti le man hinh khac. Esc khong thoat chuong trinh; dung X tren thanh tieu de de dong.

