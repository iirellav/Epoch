#include "epch.h"
#include "Font.h"
#include "MSDFData.h"
#include "Epoch/Core/Application.h"
#include "Epoch/Utils/FileSystem.h"

namespace Epoch
{
	using namespace msdf_atlas;

	struct FontInput
	{
		Buffer fontData;
		GlyphIdentifierType glyphIdentifierType;
		const char* charsetFilename;
		double fontScale;
		const char* fontName;
	};

	struct Configuration
	{
		ImageType imageType;
		msdf_atlas::ImageFormat imageFormat;
		YDirection yDirection;
		int width, height;
		double emSize;
		double pxRange;
		double angleThreshold;
		double miterLimit;
		void (*edgeColoring)(msdfgen::Shape&, double, unsigned long long);
		bool expensiveColoring;
		unsigned long long coloringSeed;
		GeneratorAttributes generatorAttributes;
	};

	#define DEFAULT_ANGLE_THRESHOLD 3.0
	#define DEFAULT_MITER_LIMIT 1.0
	#define LCG_MULTIPLIER 6364136223846793005ull
	#define LCG_INCREMENT 1442695040888963407ull
	#define THREADS 8

	namespace Utils {

		static std::filesystem::path GetCacheDirectory()
		{
			return Application::Get().GetSpecification().cacheDirectory + "/Font";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::filesystem::path cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
			{
				std::filesystem::create_directories(cacheDirectory);
			}
		}
	}

	struct AtlasHeader
	{
		uint32_t type = 0;
		uint32_t width = 0, height = 0;
	};

	static bool TryReadFontAtlasFromCache(const std::string& aFontName, float aFontSize, AtlasHeader& aHeader, void*& aPixels, Buffer& aStorageBuffer)
	{
		std::string filename = fmt::format("{}-{}.efa", aFontName, aFontSize);
		std::filesystem::path filepath = Utils::GetCacheDirectory() / filename;

		if (std::filesystem::exists(filepath))
		{
			aStorageBuffer = FileSystem::ReadBytes(filepath);
			aHeader = *aStorageBuffer.As<AtlasHeader>();
			aPixels = (uint8_t*)aStorageBuffer.data + sizeof(AtlasHeader);
			return true;
		}
		return false;
	}

	static void CacheFontAtlas(const std::string& aFontName, float aFontSize, AtlasHeader aHeader, const void* aPixels)
	{
		Utils::CreateCacheDirectoryIfNeeded();

		std::string filename = fmt::format("{}-{}.efa", aFontName, aFontSize);
		std::filesystem::path filepath = Utils::GetCacheDirectory() / filename;

		std::ofstream stream(filepath, std::ios::binary | std::ios::trunc);
		if (!stream)
		{
			stream.close();
			LOG_ERROR_TAG("Renderer", "Failed to cache font atlas to {}", filepath.string());
			return;
		}

		stream.write((char*)&aHeader, sizeof(AtlasHeader));
		stream.write((char*)aPixels, aHeader.width * aHeader.height * sizeof(float) * 4);
	}

	template <typename T, typename S, int N, GeneratorFunction<S, N> GEN_FN>
	static std::shared_ptr<Texture2D> CreateAndCacheAtlas(const std::string& aFontName, float aFontSize, const std::vector<GlyphGeometry>& aGlyphs, const FontGeometry& aFontGeometry, const Configuration& aConfig)
	{
		ImmediateAtlasGenerator<S, N, GEN_FN, BitmapAtlasStorage<T, N>> generator(aConfig.width, aConfig.height);
		generator.setAttributes(aConfig.generatorAttributes);
		generator.setThreadCount(THREADS);
		generator.generate(aGlyphs.data(), (int)aGlyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>) generator.atlasStorage();

		AtlasHeader header;
		header.width = bitmap.width;
		header.height = bitmap.height;
		CacheFontAtlas(aFontName, aFontSize, header, bitmap.pixels);

		TextureSpecification spec;
		spec.format = TextureFormat::RGBA32F;
		spec.width = header.width;
		spec.height = header.height;
		spec.debugName = "FontAtlas";
		std::shared_ptr<Texture2D> texture = Texture2D::Create(spec, bitmap.pixels);
		return texture;
	}

	static std::shared_ptr<Texture2D> CreateCachedAtlas(AtlasHeader aHeader, const void* aPixels)
	{
		TextureSpecification spec;
		spec.format = TextureFormat::RGBA32F;
		spec.width = aHeader.width;
		spec.height = aHeader.height;
		spec.debugName = "FontAtlas";
		std::shared_ptr<Texture2D> texture = Texture2D::Create(spec, aPixels);
		return texture;
	}


	Font::Font(const std::filesystem::path& aFilepath): myMSDFData(new MSDFData())
	{
		myName = aFilepath.stem().string();

		Buffer buffer = FileSystem::ReadBytes(aFilepath);
		CreateAtlas(buffer);
		buffer.Release();
	}

	Font::Font(const std::string& aName, Buffer aBuffer) :
		myName(aName), myMSDFData(new MSDFData())
	{
		CreateAtlas(aBuffer);
	}

	void Font::Init()
	{
		staticDefaultFont = std::make_shared<Font>("Resources/Fonts/opensans/OpenSans-Regular.ttf");
	}

	void Font::Shutdown()
	{
		staticDefaultFont.reset();
	}

	void Font::CreateAtlas(Buffer aBuffer)
	{
		EPOCH_PROFILE_FUNC();

		int result = 0;
		FontInput fontInput = { };
		Configuration config = { };
		fontInput.fontData = aBuffer;
		fontInput.glyphIdentifierType = GlyphIdentifierType::UNICODE_CODEPOINT;
		fontInput.fontScale = -1;
		config.imageType = ImageType::MSDF;
		config.imageFormat = msdf_atlas::ImageFormat::BINARY_FLOAT;
		config.yDirection = YDirection::BOTTOM_UP;
		config.edgeColoring = msdfgen::edgeColoringInkTrap;
		const char* imageFormatName = nullptr;
		int fixedWidth = -1, fixedHeight = -1;
		config.generatorAttributes.config.overlapSupport = true;
		config.generatorAttributes.scanlinePass = true;
		double minEmSize = 0;
		double rangeValue = 2.0;
		TightAtlasPacker::DimensionsConstraint atlasSizeConstraint = TightAtlasPacker::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE;
		config.angleThreshold = DEFAULT_ANGLE_THRESHOLD;
		config.miterLimit = DEFAULT_MITER_LIMIT;
		config.imageType = ImageType::MTSDF;
		config.emSize = 40;

		// Load fonts
		bool anyCodepointsAvailable = false;
		class FontHolder
		{
			msdfgen::FreetypeHandle* ft;
			msdfgen::FontHandle* font;
		public:
			FontHolder() : ft(msdfgen::initializeFreetype()), font(nullptr) {}
			~FontHolder()
			{
				if (ft)
				{
					if (font)
					{
						msdfgen::destroyFont(font);
					}
					msdfgen::deinitializeFreetype(ft);
				}
			}
			bool load(Buffer buffer)
			{
				if (ft && buffer)
				{
					if (font)
					{
						msdfgen::destroyFont(font);
					}
					if ((font = msdfgen::loadFontData(ft, buffer.As<const msdfgen::byte>(), int(buffer.size))))
					{
						return true;
					}
				}
				return false;
			}
			operator msdfgen::FontHandle* () const
			{
				return font;
			}
		} font;

		bool success = font.load(fontInput.fontData);
		EPOCH_ASSERT(success, "Failed to load font!");

		if (fontInput.fontScale <= 0)
		{
			fontInput.fontScale = 1;
		}

		// Load character set
		fontInput.glyphIdentifierType = GlyphIdentifierType::UNICODE_CODEPOINT;
		Charset charset;

		// From ImGui
		static const uint32_t charsetRanges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
			0x2DE0, 0x2DFF, // Cyrillic Extended-A
			0xA640, 0xA69F, // Cyrillic Extended-B
			0,
		};

		for (int range = 0; range < 8; range += 2)
		{
			for (uint32_t c = charsetRanges[range]; c <= charsetRanges[range + 1]; c++)
			{
				charset.add(c);
			}
		}

		// Load glyphs
		myMSDFData->fontGeometry = FontGeometry(&myMSDFData->glyphs);
		int glyphsLoaded = -1;
		switch (fontInput.glyphIdentifierType)
		{
			case GlyphIdentifierType::GLYPH_INDEX:
				glyphsLoaded = myMSDFData->fontGeometry.loadGlyphset(font, fontInput.fontScale, charset);
				break;
			case GlyphIdentifierType::UNICODE_CODEPOINT:
				glyphsLoaded = myMSDFData->fontGeometry.loadCharset(font, fontInput.fontScale, charset);
				anyCodepointsAvailable |= glyphsLoaded > 0;
				break;
		}

		EPOCH_ASSERT(glyphsLoaded >= 0, "No glyphs loaded!");
		LOG_DEBUG_TAG("Renderer", "Loaded geometry of {} out of {} glyphs", glyphsLoaded, (int)charset.size());
		// List missing glyphs
		if (glyphsLoaded < (int)charset.size())
		{
			LOG_WARNING_TAG("Renderer", "Missing {} {}", (int)charset.size() - glyphsLoaded, fontInput.glyphIdentifierType == GlyphIdentifierType::UNICODE_CODEPOINT ? "codepoints" : "glyphs");
		}

		if (fontInput.fontName)
		{
			myMSDFData->fontGeometry.setName(fontInput.fontName);
		}

		// Determine final atlas dimensions, scale and range, pack glyphs
		double pxRange = rangeValue;
		bool fixedDimensions = fixedWidth >= 0 && fixedHeight >= 0;
		bool fixedScale = config.emSize > 0;
		TightAtlasPacker atlasPacker;
		if (fixedDimensions)
		{
			atlasPacker.setDimensions(fixedWidth, fixedHeight);
		}
		else
		{
			atlasPacker.setDimensionsConstraint(atlasSizeConstraint);
		}
		atlasPacker.setPadding(config.imageType == ImageType::MSDF || config.imageType == ImageType::MTSDF ? 0 : -1);
		// TODO: In this case (if padding is -1), the border pixels of each glyph are black, but still computed. For floating-point output, this may play a role.
		if (fixedScale)
		{
			atlasPacker.setScale(config.emSize);
		}
		else
		{
			atlasPacker.setMinimumScale(minEmSize);
		}
		atlasPacker.setPixelRange(pxRange);
		atlasPacker.setMiterLimit(config.miterLimit);
		if (int remaining = atlasPacker.pack(myMSDFData->glyphs.data(), (int)myMSDFData->glyphs.size()))
		{
			if (remaining < 0)
			{
				EPOCH_ASSERT(false, "Failed");
			}
			else
			{
				LOG_ERROR_TAG("Renderer", "Error: Could not fit {} out of {} glyphs into the atlas.", remaining, (int)myMSDFData->glyphs.size());
				EPOCH_ASSERT(false, "Failed");
			}
		}
		atlasPacker.getDimensions(config.width, config.height);
		EPOCH_ASSERT(config.width > 0 && config.height > 0, "Failed");
		config.emSize = atlasPacker.getScale();
		config.pxRange = atlasPacker.getPixelRange();
		if (!fixedScale)
		{
			LOG_DEBUG_TAG("Renderer", "Glyph size: {} pixels/EM", config.emSize);
		}
		if (!fixedDimensions)
		{
			LOG_DEBUG_TAG("Renderer", "Atlas dimensions: {0} x {1}", config.width, config.height);
		}


		// Edge coloring
		if (config.imageType == ImageType::MSDF || config.imageType == ImageType::MTSDF)
		{
			if (config.expensiveColoring)
			{
				Workload([&glyphs = myMSDFData->glyphs, &config](int i, int threadNo) -> bool
				{
					unsigned long long glyphSeed = (LCG_MULTIPLIER * (config.coloringSeed ^ i) + LCG_INCREMENT) * !!config.coloringSeed;
					glyphs[i].edgeColoring(config.edgeColoring, config.angleThreshold, glyphSeed);
					return true;
				}, (int)myMSDFData->glyphs.size()).finish(THREADS);
			}
			else
			{
				unsigned long long glyphSeed = config.coloringSeed;
				for (GlyphGeometry& glyph : myMSDFData->glyphs)
				{
					glyphSeed *= LCG_MULTIPLIER;
					glyph.edgeColoring(config.edgeColoring, config.angleThreshold, glyphSeed);
				}
			}
		}

		// Check cache here
		Buffer storageBuffer;
		AtlasHeader header;
		void* pixels;
		if (TryReadFontAtlasFromCache(myName, (float)config.emSize, header, pixels, storageBuffer))
		{
			LOG_INFO_TAG("Renderer", "Created cached font atlas");
			myTextureAtlas = CreateCachedAtlas(header, pixels);
			storageBuffer.Release();
		}
		else
		{
			LOG_INFO_TAG("Renderer", "Created a new font atlas and cached it");

			bool floatingPointFormat = true;
			std::shared_ptr<Texture2D> texture;
			switch (config.imageType)
			{
			case ImageType::MSDF:
				if (floatingPointFormat)
				{
					texture = CreateAndCacheAtlas<float, float, 3, msdfGenerator>(myName, (float)config.emSize, myMSDFData->glyphs, myMSDFData->fontGeometry, config);
				}
				else
				{
					texture = CreateAndCacheAtlas<byte, float, 3, msdfGenerator>(myName, (float)config.emSize, myMSDFData->glyphs, myMSDFData->fontGeometry, config);
				}
				break;
			case ImageType::MTSDF:
				if (floatingPointFormat)
				{
					texture = CreateAndCacheAtlas<float, float, 4, mtsdfGenerator>(myName, (float)config.emSize, myMSDFData->glyphs, myMSDFData->fontGeometry, config);
				}
				else
				{
					texture = CreateAndCacheAtlas<byte, float, 4, mtsdfGenerator>(myName, (float)config.emSize, myMSDFData->glyphs, myMSDFData->fontGeometry, config);
				}
				break;
			}

			myTextureAtlas = texture;
		}
	}
};