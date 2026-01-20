# Design File — TSS_App (PhotoApp)

**Project:** TSS_App  
**Technology:** C++20, Qt 6 (Widgets / Gui / Core)  
**Platform:** Windows (MSVC 2022)  
**Source structure (key classes):** `TSS_project`, `DataStorage`, `ImageEditor`, `MetaDataStorage`, `ClickableImgs`, `PhotoMeta`

---

## 1. Overview

### 1.1 Purpose

This document describes how the application is designed and how it implements the requirements from **SRS_PhotoApp.md** and the assignment (FinalneZadanie.pdf).  
Goal: keep the design and SRS **consistent** (same terms, IDs, names).

### 1.2 Scope (what the app does)

- Opens a folder with photos (including subfolders) and shows them as a gallery (thumbnails + paging).
- Displays a selected photo and allows editing (rotate, crop, filters, adjustments, watermark).
- Allows tagging and rating each photo.
- Saves tag/rating locally and restores it after restart (persistence).
- Exports an image via “Save As” into supported formats.

### 1.3 Key constraints

- Desktop Qt Widgets application
- Local-only data (no network)
- Works with large folders (scan is not blocking UI)

---

## 2. High-Level Design

### 2.1 Architecture (simple layered view)

**UI layer (presentation):**

- `TSS_project` (QMainWindow + slots for UI actions)
- Qt Designer UI (`ui_TSS_project`)
- `ClickableImgs` (thumbnail item that can be clicked)

**Application logic / services:**

- `DataStorage` — photo record + scanning + sorting/filtering algorithms
- `ImageEditor` — image processing operations (rotate, filters, adjustments, watermark, crop helpers)
- `MetaDataStorage` — persistence of tag/rating to JSON

**Data sources:**

- File system (photos in folders)
- Local JSON storage (`photo_meta.json` in AppDataLocation)

---

### 2.2 Component Responsibilities

**TSS_project**

- Responsibility: Main window (UI layer). Orchestrates user actions: open folder, build gallery pages, open photo, display photo, call edit operations, call export/save, update tag/rating UI.
- Communicates with:
  - `DataStorage` (gets/updates current list of photos, filtering/sorting, page building)
  - `ImageEditor` (performs edits on `QImage`)
  - `MetaDataStorage` (load/save/restore tag & rating across sessions)
  - Qt Widgets (`QGraphicsView/QGraphicsScene`, dialogs, buttons, comboboxes, status bar)
- Supports SRS: FR-1.1–FR-1.7 (import/open/display/scroll), FR-1.8–FR-1.10 (tag/rating/filter/sort), FR-2.1/FR-2.3–FR-2.7 (edit/filter/watermark/export), FR-3.2 (progress UI), FR-3.4.

**DataStorage**

- Responsibility: Represents one photo record (path/name/date/tag/rating). Provides folder scanning (including subfolders), and in-memory operations: merge sort by date/rating and filtering by tag.
- Communicates with:
  - `TSS_project` (UI requests scan/sort/filter and uses resulting `QVector<DataStorage>`)
  - `MetaDataStorage` (during scan to attach saved tag/rating to records)
- Supports SRS: FR-1.1–FR-1.2 (load from folder/subfolders), FR-1.4–FR-1.6 (gallery + open photo via index), FR-1.8–FR-1.10 (tag/rating/filter/sort), FR-1.11 (large libraries – data structure side).

**ImageEditor**

- Responsibility: Application logic for editing. Implements operations on images: rotate, crop workflow, brightness/contrast/saturation adjustments, filters (monochrome/sepia/pastel/vintage/negative), watermark/logo (with opacity and corner position). Input/output is `QImage` (no UI).
- Communicates with:
  - `TSS_project` (called from UI slots; returns edited `QImage` or modifies `QImage&` depending on method)
  - Qt Gui/Core only (`QImage`, `QColor`, etc.; for crop also receives `QGraphicsPixmapItem*` for coordinate mapping)
- Supports SRS: FR-2.1 (crop + basic adjustments + rotate), FR-2.3 (filters), FR-2.4 (apply edits to single image), FR-2.5–FR-2.6 (watermark/logo), FR-2.7 (export of edited/non-edited images through UI flow).

**MetaDataStorage**

- Responsibility: Persistence layer for tag & rating. Stores/loads metadata into local JSON, and restores metadata after rename/move using fingerprint (`quickFingerprint`) and file size.
- Communicates with:
  - `TSS_project` (calls `load()/save()`; uses `setInRecords/getFromRecords`)
  - `DataStorage` (uses metadata during scanning to populate record fields)
  - Qt Core (JSON + file IO via `QFile`, `QJsonDocument`, `QStandardPaths`)
- Supports SRS: FR-1.8–FR-1.9 (tag/rating per photo), FR-3.4 (auto-save mechanism idea), NFR-2 (persistence across sessions).

**ClickableImgs**

- Responsibility: UI helper for gallery thumbnails. Emits `doubleClicked(QGraphicsPixmapItem*)` when user double-clicks a thumbnail item.
- Communicates with:
  - `TSS_project` (connects signal to open/display selected photo)
  - Qt Graphics View framework (`QGraphicsPixmapItem`, `QGraphicsSceneMouseEvent`)
- Supports SRS: FR-1.4 (gallery view), FR-1.6 (open photo from gallery).

---

## 3. Detailed Design of One Feature (Low-level design)

### Feature chosen: Edit Photo (rotate / crop / adjustments / filters / watermark)

This section describes how photo editing is implemented internally based on the existing classes `TSS_project` and `ImageEditor`.

### 3.1 Class Diagram (UML)

**UML Class Diagram** for this feature.

![Class Diagram](diag/EdttingImg.drawio.png)

### 3.2 Class Descriptions

#### TSS_project (edit orchestration in UI layer)

- Role: Owns the currently shown image, reacts to UI events (buttons/toggles), calls `ImageEditor` operations, then refreshes the viewer. Also manages the crop UI mode (rubber-band selection) via `eventFilter()`.
- Key attributes (from implementation concept):
  - `QImage currentImage` — working image (updated after every edit).
  - `QImage currentImageOrigin` — snapshot of image before the first edit (used to reset/compare).
  - `bool isEditing` — whether edits were applied on current image.
  - Crop state: `bool isCropInProgress`, `QRubberBand* cropBand`, `QPoint cropStart`, `QRect cropRectFromView`.
  - Viewer: `QGraphicsScene* scene`, `QGraphicsPixmapItem* item` (pixmap item of the displayed image).
  - `ImageEditor editor` — edit engine used by UI.
- Main methods (edit-related):
  - `displayImage(const QImage& img)` — shows `img` in `QGraphicsView` (for FR-1.7 final behavior: image at original resolution; if larger than view, scrollbars allow navigation).
  - `bool eventFilter(QObject* obj, QEvent* ev)` — when crop mode is active, captures mouse press/move/release on `graphicsView->viewport()`, updates `cropBand` and stores selection in `cropRectFromView`.
  - Edit slots (examples directly from code):
    - Rotation: `on_leftRotation_clicked()`, `on_rightRotation_clicked()` → `editor.rotate(...)` → assign `currentImage` → `displayImage(...)`.
    - Adjustments: `on_minusBrightnessBtn_clicked() / on_plusBrightnessBtn_clicked()` → `editor.changeBrightness(...)` (in-place) → status bar message → `displayImage(...)`. Same pattern for contrast and saturation.
    - Filters: `on_monochromeBtn_toggled(bool)`, `on_sepiaBtn_toggled(bool)`, `on_pastelBtn_toggled(bool)`, `on_vintageBtn_toggled(bool)`, `on_negativeBtn_toggled(bool)` → calls `editor.apply*Filter(...)` or `editor.cleanStyleEditor(...)`, enables/disables other filter buttons, refreshes view.
    - Crop workflow:
      - `on_cropBtn_toggled(bool)` → sets `isCropInProgress`, calls `editor.startCrop(currentImage)`, enables crop buttons and disables other edit buttons.
      - `on_applyCropBtn_clicked()` → converts selected rect to scene coordinates and calls `editor.crop(sceneSel, currentImage, item)`.
      - `on_undoCropBtn_clicked()` → `editor.resetCrops(currentImage)` and refresh.
      - `on_cancelCropBtn_clicked()` → hides rubber-band and clears selection.
    - Watermark: `on_watermarkBtn_clicked()` + related UI handlers (opacity/position/off) → uses `editor.setWatermark(...)`, `editor.setWatermarkPos(...)`, `editor.applyWatermark(currentImage)`, `editor.offWatermark(currentImage)`.

#### ImageEditor (editing engine / application logic)

- Role: Implements image processing independently of UI. Most operations mutate a provided `QImage&` (adjustments/filters/watermark/crop) or return a new `QImage` (rotation).
- Key attributes:
  - Color-edit base: `QImage forColorEditsBase` and state fields `brightnessСhange`, `contrastСhange`, `saturationСhange`.
  - Filter state flags: `isMonochrome`, `isNegative`, `isSepia`, `isVintage`, `isPastel`.
  - Crop state: `QImage cropBase`, `QVector<QRect> cropHistory`.
  - Watermark state: `QImage watermark`, `int watermarkOpacity`, `QString watermarkPosition`, `bool isWatermarkSelected`.
- Main public methods (directly from the header):
  - `QImage rotate(bool isLeft, const QImage& src)` — rotates left/right (used by UI rotate buttons).
  - Adjustments:
    - `QString changeBrightness(int amount, QImage& src)`
    - `QString changeContrast(int amount, QImage& src)`
    - `QString changeSaturation(int amount, QImage& src)`  
      These update internal adjustment state and re-apply edits to `src`, returning a short message for the status bar.
  - Filters: `applyMonochromeFilter`, `applySepiaFilter`, `applyPastelFilter`, `applyVintageFilter`, `applyNegativeFilter`.
  - Crop workflow:
    - `void startCrop(const QImage& src)` — stores crop base and resets crop history.
    - `QString crop(const QRectF& sceneSel, QImage& src, QGraphicsPixmapItem* item)` — maps selection from scene to image pixels, crops `src`, stores crop history.
    - `QString resetCrops(QImage& src)` — returns to `cropBase`.
    - `void endCrop()` — clears crop state.
  - Watermark:
    - `QString setWatermark(const QString& absPath)` — loads watermark image.
    - `void setWatermarkOpacity(int o)`, `void setWatermarkPos(const QString& pos)`
    - `QString applyWatermark(QImage& src)`, `QString offWatermark(QImage& src)`
  - Reset helpers: `cleanEditor()` (reset adjustments), `cleanStyleEditor(QImage&)` (remove applied filters from current image, based on stored base/state).

- Key internal method: `QString applyColorEdits(QImage& src)` — central pipeline that rebuilds the _current preview image_ from the stored base (`forColorEditsBase`) and then applies all active edit parameters in one pass.
  - Purpose: guarantees that brightness/contrast/saturation + style filters + negative + watermark are applied consistently in a single deterministic order, without accumulating rounding artifacts from repeated edits on already-edited pixels.
  - Input/Output:
    - Uses `forColorEditsBase` as the **source of truth** (base image for color edits).
    - Produces a new intermediate `QImage img` and finally assigns it back to `src`.
    - Returns a short **status message** (empty or warning) that the UI can show in the status bar.
  - Processing steps (exact order in code):
    1. **Pre-checks**:
       - If `forColorEditsBase` is null → returns warning and does nothing.
       - Computes `totalS = w*h`; if 0 → returns warning.
    2. **Prepare correction coefficients**:
       - Contrast factor `c = (259*(contrastChange+255)) / (255*(259-contrastChange))`.
       - Saturation scale `kSat = 1 + saturationChange/255`.
    3. **Per-pixel loop (scanLine for performance)**:
       - Reads RGBA channels. Keeps alpha `a` unchanged.
       - Tracks pixel “border clipping” using `isPixOnBorder(p)` (before and after) to compute `oldClipPix/newClipPix/dClipPix`.
       - Applies edits in this order:
         - **Contrast** per channel around mid-point 128: `(channel-128)*c + 128` → clamped by `toFitChannelRange`.
         - **Saturation**: computes luminance `Y = 0.299*r + 0.587*g + 0.114*b` and scales distance from grayscale line: `Y + (channel-Y)*kSat`.
         - **Brightness**: adds `brightnessChange` to each channel.
         - **Style filter** depending on `styleFilter`:
           - `1` monochrome (sets all channels to luminance)
           - `2` sepia (warm brown transform)
           - `3` pastel (soft saturation + lift to white + contrast compression)
           - `4` vintage (desatur + warm tint + gamma lift + vignette by distance to center)
         - **Negative** if `isNegative` is enabled: `channel = 255 - channel`.
       - Writes back `qRgba(r,g,b,a)` to the pixel row.
    4. **Clipping warning message**:
       - After processing, computes `oldClipPix/newClipPix/dClipPix`.
       - If `dClipPix > 0.05` or `newClipPix > 0.7`, returns a warning string like  
          `"WARNING (too many pixels on "border") : Clipping: ..."`  
         (this is used as a quality indicator when edits push many pixels to 0/255).
    5. **Watermark layer (final stage)**:
       - If `isWatermarkApplied` and `watermarkBase` is not null → calls `applyWatermarkLayer(img)`.
       - If watermark returns a message, it is appended to the existing warning message.
    6. **Commit result**:
       - If `img` is valid, assigns `src = img`.
       - Returns the final message (empty string if everything is normal).
  - Why this is the “main” edit function:
    - UI-level edit buttons typically update internal parameters (`brightnessChange`, `contrastChange`, `saturationChange`, `styleFilter`, `isNegative`, watermark flags).
    - `applyColorEdits()` then regenerates the final image from the base in one pass, ensuring consistent output and preventing “double-application” issues.

Internal edit flow (typical):

1. `TSS_project` ensures `currentImage` is ARGB32 when needed and stores `currentImageOrigin` when the first edit starts (`isEditing` transition).
2. UI slot calls one `ImageEditor` method (rotate / crop / adjustment / filter / watermark).
3. `TSS_project` updates `currentImage`, optionally shows editor message in status bar, then calls `displayImage(currentImage)`.

---

## 4. Requirements → Design Mapping

### Functional Requirements

| Req ID                           | Implemented by (design elements)                                                                         |
| -------------------------------- | -------------------------------------------------------------------------------------------------------- |
| FR-1.1 Open folder               | `TSS_project::on_actionOpen_folder_triggered()`, `scanFolderWithProgress()`, `DataStorage::scanFolder()` |
| FR-1.2 Include subfolders        | `DataStorage::scanFolder()` (recursive traversal)                                                        |
| FR-1.3 Supported formats         | Qt image plugins + `QImageReader/QImage` (format by extension + plugins deployed)                        |
| FR-1.4 Gallery                   | `TSS_project` gallery UI + `ClickableImgs` + paging (`buildCurrentPage()`, `showPage()`)                 |
| FR-1.5 Thumbnails adapt          | `TSS_project` paging/thumbnail layout logic (`checkNumOfImg()` + page rebuild)                           |
| FR-1.6 View selected photo       | thumbnail click → `TSS_project` loads `QImage` → `displayImage()`                                        |
| FR-1.7 Original size + scroll    | `displayImage()` sets scene rect to full image; scrollbars in `QGraphicsView`                            |
| FR-1.8 Tag & rating              | UI combo boxes → slot updates + `MetaDataStorage::setInRecords()`                                        |
| FR-1.9 One tag, rating 1–5       | Enforced by UI controls + `PhotoMeta.rating/tag` storage                                                 |
| FR-1.10 Search/filter            | `filterCurrentIndexChanged()`, `DataStorage::mergeSort()` + filter logic                                 |
| FR-1.11 Large libraries          | background scan (`QtConcurrent`), paging, load image on demand                                           |
| FR-2.1 Crop/adjust/rotate        | `ImageEditor` methods + `TSS_project` buttons/slots                                                      |
| FR-2.3 Filters                   | `ImageEditor` filter functions (sepia/negative/etc.)                                                     |
| FR-2.4 Apply edits to selected   | `TSS_project` holds `currentImage/currentImgPath` per selection                                          |
| FR-2.5 Watermark                 | `ImageEditor` watermark method + UI (opacity/enable/disable)                                             |
| FR-2.6 Watermark image from file | UI file dialog selects watermark file; passed into `ImageEditor`                                         |
| FR-2.7 Export in formats         | `TSS_project::saveCurrentImageAs()` (`QImage::save`)                                                     |
| FR-3.1 Clear UI                  | Qt Widgets layout + actions                                                                              |
| FR-3.2 Progress bar              | `runWithBusyBar()` adds `QProgressBar` to status bar                                                     |
| FR-3.3 Easy start                | CMake build + `windeployqt` deployment                                                                   |
| FR-3.4 Auto-save tag/rating      | `MetaDataStorage.save/load` + UI updates to records                                                      |
| FR-3.5 Weekly updates            | Process requirement (not code); tracked in project workflow                                              |

### Non-Functional Requirements

| NFR ID            | Implemented by                                                |
| ----------------- | ------------------------------------------------------------- |
| NFR-1 Performance | async scanning + paging; avoid loading all images at once     |
| NFR-2 Persistence | `MetaDataStorage` JSON + fingerprint recovery                 |
| NFR-3 Usability   | direct buttons/menus; common operations are short paths in UI |
| NFR-4 Portability | Windows + Qt 6                                                |
| NFR-5 Security    | local-only storage; no network transmission                   |

---

## 5. Technical Constraints and Decisions

### Decision 1 — Store only paths/names, not all images in RAM

- I do **not** keep all photos loaded in memory. In the "database" I store only `QString imgPath` and `QString imgName` (plus lightweight derived fields like date/tag/rating inside `DataStorage`).
- Why: this is fully enough to locate a file and work with it, and it gives a big win in **RAM usage** and **performance** compared to holding many full `QImage` objects at once.
- Practical effect: the app can handle large folders, because it loads the actual `QImage` only when the user opens a specific photo.

### Decision 2 — Local persistence via `photo_meta.json` + fingerprint recovery

- Tag and rating are saved locally in `photo_meta.json` (stored in `QStandardPaths::AppDataLocation`) using `MetaDataStorage`.
- To restore metadata even if a user **renames or moves** the photo, the app computes a lightweight **fingerprint** (`quickFingerprint`) from file size + first/last chunks and uses it as a key to recover the correct record.
- Why: this keeps persistence reliable across sessions without a heavy database and without scanning full file contents.

### Decision 3 — ImageEditor uses a “base + parameters + re-render” pipeline

- In `ImageEditor`, operations that change image geometry (like **crop** and **rotate**) update the current working image and also update the relevant “base” state used for further edits.
- Color/style edits (brightness/contrast/saturation + filters + negative + watermark) are not applied endlessly on top of already-edited pixels. Instead, the editor stores:
  - a base image (`forColorEditsBase`),
  - edit parameters (brightness/contrast/saturation, selected style filter, negative, watermark settings),
    and then one central method (`applyColorEdits`) **re-renders** the current image from the base in a fixed order.
- Why: this makes the behavior predictable and reversible. Even if pixels got clipped to 0/255 after a strong adjustment, the user can “go back” by decreasing the parameter, because the image is rebuilt from the base, not from the clipped result.

---

## 6. Open Risks / Questions

- **Undo/Discard edge case in single-photo gallery mode (time constraint):**  
  After editing an image, if the user uses the “discard/undo” button while the gallery is in a “single photo” display mode, the current implementation can end up in a state where **tag changes and image edits are effectively blocked for that photo until the user switches away/reloads it**.  
  This is a known UI/state-management edge case that was discovered very late in development, and there was not enough time left to fix it properly.

- **Cleaner UI vs. application logic separation (possible controller layer):**  
  For a stricter Clean Code architecture and a thinner UI layer, it would be better to introduce an additional layer (e.g., `GalleryController` / `AppController`). The idea is that `TSS_project` would only react to signals and forward user intent to the controller, while the controller would:
  - coordinate `DataStorage` operations (scan, sort, filter),
  - build gallery pages,
  - manage state transitions consistently,
  - and return ready-to-display results back to the UI.  
    This would not change functionality, but it would improve maintainability and make it easier to keep **unit/integration tests** separate from **GUI tests**. I realized this opportunity late (after most functionality was implemented), and due to time constraints I decided not to perform this refactor. The core application logic is already extracted into technical classes (`DataStorage`, `ImageEditor`, `MetaDataStorage`), so the current structure still meets the “separation” requirement for the project.

---

## 7. Summary

The application is structured as a Qt Widgets UI layer (`TSS_project` + Designer UI + `ClickableImgs`) that orchestrates user actions, and a small set of technical components that implement the core logic: `DataStorage` for scanning and in-memory sorting/filtering, `ImageEditor` for all image processing, and `MetaDataStorage` for local persistence of tag/rating.  
In detail, this design file describes the **Edit Photo** feature, including the interaction between `TSS_project` (UI event handlers + crop mode) and `ImageEditor` (rotate/crop/filters/adjustments/watermark pipeline), and references a UML class diagram for this feature.  
The design satisfies the SRS because each requirement maps cleanly to a specific component: UI requirements are handled in `TSS_project`, image-editing requirements are implemented in `ImageEditor`, and metadata persistence requirements are implemented in `MetaDataStorage` with JSON + fingerprint recovery.  
This separation keeps the UI mostly orchestration-focused while the real algorithms live in testable classes, which also supports unit, integration, and GUI testing as required by the assignment.
