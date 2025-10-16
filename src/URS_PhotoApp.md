# URS-lite — Photo Management & Editing App (Windows, C++/Qt)

## 1. Vision

Build a simple, fast Windows application for **browsing, organizing, and lightly editing photos**. It should be easy to run **without installation** (portable) or have a **very simple installer**. The UI must be clean and modern so non‑technical users can use it immediately.

---

## 2. Stakeholders

| Stakeholder                                  | Needs / Goals                                                                                                                  | Influence |
| -------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------ | --------- |
| Customer / Sponsor                           | Create an app, where he can browse albums, do quick edits, tag/search; regular progress visibility; delivery by end of January | High      |
| End Users (family/team handling many photos) | Import lots of photos (incl. HEIC, RAW), view fast, basic edits, batch watermark, tagging, sorting                             | High      |
| Developer (student)                          | Wants to make good project, but also in short time, and get new experience                                                     | High      |
| Project Manager (teacher)                    | Wants to get info about progress every week, and inform about new technologies, that will be used during developing            | High      |
| Department Leader                            | Expects the developer to acquire the necessary skills during the project development process.                                  | Low       |

---

## 3. User Types / Personas

- **Persona 1:** - Wants to import a phone album, quickly find “blue photos from July”, rate and tag, and export copies.
- **Persona 2:** Has many files including **.jpg, .png, .tiff, .RAW**; needs batch watermark and do quick edits; wants crash‑safe processing.

---

## 4. User Stories (Backlog) with MoSCoW

### Must (M)

1. As a user, I can **run the app on Windows** without a complex install (portable EXE or simple installer).
2. As a user, I can **import photos from folders/external disks**, including **.jpg, .png, .tiff**, paired **RAW**, and **.heic**.
3. As a user, I can **browse albums** and **open a single photo**.
4. As a user, I can do **basic edits**, like **cutting**, **rotate**, **brightness**, **contrast**, **sharpness**.
5. As a user, I can **export** edited photos to common formats (**.jpg, .png, .tiff**).
6. As a user, I can **add a watermark/logo** (choose place and transparency).
7. As a user, I can **edit a group of photos** by selection and also process **a single image**.
8. As a user, I can **tag**, **rate**, and **comment** photos; I can **search/filter** by date, tag, rating.
9. As a user, I can see a **progress bar** during long operations so I know the app hasn’t frozen.
10. As a user, my work is **auto‑saved** so if the app crashes, changes and queue settings are not lost.
11. As developer, I must show **weekly progress** to the customer.
12. As a user,I can **edit categories** of photos.
13. Performance: The app **loads a library of 10,000 photos quickly** and **does not consume excessive memory**.

### Should (S)

1. The app **suggests categories automatically**.
2. **Dark mode** for comfortable viewing.

### Could (C)

1. Include **preset filters** (e.g., “Warm”, “Cool”, “Vintage”).

### Won’t (W) — this release

1. Project won't have AI editing tool.

---

## 5. Acceptance Criteria (for key Must stories)

**Story:** As a developer, I want to inform customer, that project is done.

- AC-1: Project possess all of the **basic features** (cut, rotation, contrast, brightness, ...).
- AC-2: Project must to run **fast**, and also do **not** consume a **lot** of system **resourses**.
- AC-3: Images can be easily imported and exported in different formats, as **.jpg, .png, .tiff**; opens sidecar/paired **RAW**; from phone in **.HEIC**
- AC-4:Import from local folders and **external drives** is supported.
- AC-5: There will be possibility of adding watermark to image in different **corners** and **opacity**.
- AC-6: Images can be sorted by **category**, **date** and **tags** (category and tags should be editible), **rating**, **comments**.
- AC-7: Posibility to process many **images** at once, or just **one**.
- AC-8: **Autosave** queue and unsaved edits at least every **30s** and on major actions.
- AC-9: First thumbnail grid for a library of **10,000 photos appears within ≤ 10 seconds**.
- AC-10: Project release before january end.

---

## 6. BDD Scenarios (Given–When–Then)

**Story:** Rotate image clockwise  
_Given_ a photo is open in the editor  
_When_ the user clicks **Rotate 90° CW**  
_Then_ the image is rotated 90° clockwise and the **Undo** action becomes available

**Story:** Importing 10,000 photos  
_Given_ a folder with 10,000 mixed **.jpg/.png/.tiff/.heic** images  
_When_ the user chooses **Import Folder**  
_Then_ thumbnails begin to appear within **3 seconds**, and the full grid is usable (scrollable/filterable) within **10 seconds**

**Story:** Batch watermark  
_Given_ the user selected 1,000 photos and configured a **bottom‑right** watermark with **40% opacity**  
_When_ the user clicks **Start Batch**  
_Then_ the app processes all photos, shows a **progress bar**, and exports to the chosen folder.

---

## 7. Non‑functional Requirements (NFRs)

- **Platform:** Windows. Portable EXE or one‑click installer.
- **Performance:** See AC‑9 (10k photos visible ≤ 10 s).
- **Reliability:** Autosave every **30 s**; recovery after crash.
- **Usability:** Clean, modern UI; visible progress indicators for long tasks.
- **Security/Privacy:** Never overwrite source files; edited outputs saved to a separate folder by default.
- **Licensing:** Use **free/open‑source libraries** with compatible licenses (e.g., image codecs, HEIC/RAW). Keep a NOTICE file.

---

## 8. Out of Scope (Won't have this time)

- Editing images with AI
- Mobile version of application

---

## 9. Open Questions and Assumptions

- Q1: Is extending of deadline is possible?
- Q2: Is using AI for automatic categorizing is necessarly?
- Assumption 1: Cutomer provided most of requests, and developer understood, what the customer wants.
- Assuption 2: All of the needed software for developing works fine
