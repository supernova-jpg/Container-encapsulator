# Pro Muxer Icon Creation Guide

## Icon Design Concept

**Pro Muxer** needs a professional icon that represents video stream muxing/remuxing. Here are the design recommendations:

### Visual Elements
1. **Main Symbol**: A container/box with video stream arrows
2. **Color Scheme**: 
   - Primary: Deep blue (#1e3a8a) - representing professionalism
   - Secondary: Orange/amber (#f59e0b) - representing media/video
   - Accent: White/light gray for contrast

### Icon Concept Description
```
[Container Icon Design]
┌─────────────────────┐
│  ╔═══╗    ╔═══╗     │  Video containers (MP4, MKV, etc.)
│  ║MP4║ -> ║MKV║     │  Arrow showing conversion
│  ╚═══╝    ╚═══╝     │
│                     │
│  ┌─┐ ┌─┐ ┌─┐       │  Stream representations
│  └─┘ └─┘ └─┘       │  (Video, Audio, Subtitle)
│                     │
│      ProMuxer       │  App name (optional for small sizes)
└─────────────────────┘
```

### Required Icon Sizes

Create the following icon sizes for Windows:
- 256x256 (high resolution)
- 128x128 (standard)
- 64x64 (medium)
- 48x48 (normal)
- 32x32 (small)
- 16x16 (tiny)

### How to Create the Icon

#### Option 1: Using Free Online Tools
1. Go to **Canva** or **GIMP** (free)
2. Create a 256x256px canvas
3. Use the concept above to design the icon
4. Export as PNG
5. Use **ConvertICO.com** to convert PNG to ICO format

#### Option 2: Using Professional Tools
1. Adobe Illustrator or Photoshop
2. Create vector-based design for scalability
3. Export multiple sizes
4. Combine into .ico file

#### Option 3: AI-Generated (Recommended)
Use AI image generators like:
- **DALL-E**: "Professional software icon for video muxing tool, container with media streams, blue and orange colors, modern flat design"
- **Midjourney**: "App icon, video container conversion, professional software UI, flat design, blue orange color scheme"
- **Stable Diffusion**: "Software application icon, video muxer, container format conversion, professional design"

### Icon File Location
Once created, save the icon as:
```
resources/app.ico
```

### Temporary Placeholder
Until the real icon is created, the application will use the default Qt application icon.

## Implementation Status
- [x] Resource file structure created
- [x] Windows resource (.rc) file prepared
- [ ] Actual app.ico file needs to be created
- [ ] Test icon in compiled application

## Notes for Developer
The Windows resource file `resources/app.rc` is ready and references `app.ico`. Once you create the actual icon file, uncomment the RC_FILE line in `ProMuxer.pro` and the application will use your custom icon.