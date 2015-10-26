#include <iostream>
#include <tuple>
#include <cstring>
#include "index.h"

using namespace std;

extern unsigned char ux_debug_js[];
extern unsigned char ext_all_debug_js[];
extern unsigned char resources_svg_font_awesome__fonts__fontawesome_webfont_svg[];
extern unsigned char resources_svg_font_ext__fonts__ExtJS_svg[];
extern unsigned char resources_otf_font_awesome__fonts__FontAwesome_otf[];
extern unsigned char resources_eot_font_ext__fonts__ExtJS_eot[];
extern unsigned char resources_eot_font_awesome__fonts__fontawesome_webfont_eot[];
extern unsigned char resources_css_classic__theme_triton__resources__theme_triton_all_debug_1_css[];
extern unsigned char resources_css_classic__theme_triton__resources__theme_triton_all_debug_2_css[];
extern unsigned char resources_css_ux__ux_all_debug_css[];
extern unsigned char resources_woff_font_awesome__fonts__fontawesome_webfont_woff[];
extern unsigned char resources_woff_font_ext__fonts__ExtJS_woff[];
extern unsigned char resources_woff2_font_awesome__fonts__fontawesome_webfont_woff2[];
extern unsigned char resources_ttf_fonts__OpenSans_BoldItalic_ttf[];
extern unsigned char resources_ttf_fonts__OpenSans_SemiboldItalic_ttf[];
extern unsigned char resources_ttf_fonts__OpenSans_Italic_ttf[];
extern unsigned char resources_ttf_fonts__OpenSans_ExtraBoldItalic_ttf[];
extern unsigned char resources_ttf_font_ext__fonts__ExtJS_ttf[];
extern unsigned char resources_ttf_fonts__OpenSans_ExtraBold_ttf[];
extern unsigned char resources_ttf_fonts__OpenSans_Regular_ttf[];
extern unsigned char resources_ttf_fonts__OpenSans_Semibold_ttf[];
extern unsigned char resources_ttf_fonts__OpenSans_Bold_ttf[];
extern unsigned char resources_ttf_fonts__OpenSans_LightItalic_ttf[];
extern unsigned char resources_ttf_font_awesome__fonts__fontawesome_webfont_ttf[];
extern unsigned char resources_ttf_fonts__OpenSans_Light_ttf[];
extern unsigned char resources_js_classic__locale__locale_ukr_debug_js[];
extern unsigned char resources_js_classic__locale__locale_ru_debug_js[];
extern unsigned char resources_js_classic__locale__locale_en_debug_js[];
extern unsigned char resources_favicon_ico[];

extern unsigned int ux_debug_js_len;
extern unsigned int ext_all_debug_js_len;
extern unsigned int resources_svg_font_awesome__fonts__fontawesome_webfont_svg_len;
extern unsigned int resources_svg_font_ext__fonts__ExtJS_svg_len;
extern unsigned int resources_otf_font_awesome__fonts__FontAwesome_otf_len;
extern unsigned int resources_eot_font_ext__fonts__ExtJS_eot_len;
extern unsigned int resources_eot_font_awesome__fonts__fontawesome_webfont_eot_len;
extern unsigned int resources_css_classic__theme_triton__resources__theme_triton_all_debug_1_css_len;
extern unsigned int resources_css_classic__theme_triton__resources__theme_triton_all_debug_2_css_len;
extern unsigned int resources_css_ux__ux_all_debug_css_len;
extern unsigned int resources_woff_font_awesome__fonts__fontawesome_webfont_woff_len;
extern unsigned int resources_woff_font_ext__fonts__ExtJS_woff_len;
extern unsigned int resources_woff2_font_awesome__fonts__fontawesome_webfont_woff2_len;
extern unsigned int resources_ttf_fonts__OpenSans_BoldItalic_ttf_len;
extern unsigned int resources_ttf_fonts__OpenSans_SemiboldItalic_ttf_len;
extern unsigned int resources_ttf_fonts__OpenSans_Italic_ttf_len;
extern unsigned int resources_ttf_fonts__OpenSans_ExtraBoldItalic_ttf_len;
extern unsigned int resources_ttf_font_ext__fonts__ExtJS_ttf_len;
extern unsigned int resources_ttf_fonts__OpenSans_ExtraBold_ttf_len;
extern unsigned int resources_ttf_fonts__OpenSans_Regular_ttf_len;
extern unsigned int resources_ttf_fonts__OpenSans_Semibold_ttf_len;
extern unsigned int resources_ttf_fonts__OpenSans_Bold_ttf_len;
extern unsigned int resources_ttf_fonts__OpenSans_LightItalic_ttf_len;
extern unsigned int resources_ttf_font_awesome__fonts__fontawesome_webfont_ttf_len;
extern unsigned int resources_ttf_fonts__OpenSans_Light_ttf_len;
extern unsigned int resources_js_classic__locale__locale_ukr_debug_js_len;
extern unsigned int resources_js_classic__locale__locale_ru_debug_js_len;
extern unsigned int resources_js_classic__locale__locale_en_debug_js_len;
extern unsigned int resources_favicon_ico_len;

pair<string, tuple<unsigned char*, unsigned int, string> > extfs_items[] = 
{
    // Main modules
    make_pair("/ext-all-debug.js",                                            make_tuple(ext_all_debug_js, ext_all_debug_js_len, "text/javascript")),
    make_pair("/ux-debug.js",                                                 make_tuple(ux_debug_js, ux_debug_js_len, "text/javascript")),

    // Stylesheet
    make_pair("/ux/ux-all-debug.css",                                         make_tuple(resources_css_ux__ux_all_debug_css, resources_css_ux__ux_all_debug_css_len, "text/css")),
    make_pair("/classic/theme-triton/resources/theme-triton-all-debug_1.css", make_tuple(resources_css_classic__theme_triton__resources__theme_triton_all_debug_1_css, resources_css_classic__theme_triton__resources__theme_triton_all_debug_1_css_len, "text/css")),
    make_pair("/classic/theme-triton/resources/theme-triton-all-debug_2.css", make_tuple(resources_css_classic__theme_triton__resources__theme_triton_all_debug_2_css, resources_css_classic__theme_triton__resources__theme_triton_all_debug_2_css_len, "text/css")),

    // Locales
    make_pair("/classic/locale/locale-en-debug.js",                           make_tuple(resources_js_classic__locale__locale_en_debug_js, resources_js_classic__locale__locale_ukr_debug_js_len, "text/javascript")),
    make_pair("/classic/locale/locale-ru-debug.js",                           make_tuple(resources_js_classic__locale__locale_ru_debug_js, resources_js_classic__locale__locale_ukr_debug_js_len, "text/javascript")),
    make_pair("/classic/locale/locale-ukr-debug.js",                          make_tuple(resources_js_classic__locale__locale_ukr_debug_js, resources_js_classic__locale__locale_ukr_debug_js_len, "text/javascript")),

    // Icon
    make_pair("/favicon.ico",                                                 make_tuple(resources_favicon_ico, resources_favicon_ico_len, "image/vnd.microsoft.icon")),

    // Fonts
    make_pair("/fonts/OpenSans-BoldItalic.ttf",                               make_tuple(resources_ttf_fonts__OpenSans_BoldItalic_ttf, resources_ttf_fonts__OpenSans_BoldItalic_ttf_len, "application/octet-stream")),
    make_pair("/fonts/OpenSans-Bold.ttf",                                     make_tuple(resources_ttf_fonts__OpenSans_Bold_ttf, resources_ttf_fonts__OpenSans_Bold_ttf_len, "application/octet-stream")),
    make_pair("/fonts/OpenSans-ExtraBoldItalic.ttf",                          make_tuple(resources_ttf_fonts__OpenSans_ExtraBoldItalic_ttf, resources_ttf_fonts__OpenSans_ExtraBoldItalic_ttf_len, "application/octet-stream")),
    make_pair("/fonts/OpenSans-ExtraBold.ttf",                                make_tuple(resources_ttf_fonts__OpenSans_ExtraBold_ttf, resources_ttf_fonts__OpenSans_ExtraBold_ttf_len, "application/octet-stream")),
    make_pair("/fonts/OpenSans-Italic.ttf",                                   make_tuple(resources_ttf_fonts__OpenSans_Italic_ttf, resources_ttf_fonts__OpenSans_Italic_ttf_len, "application/octet-stream")),
    make_pair("/fonts/OpenSans-LightItalic.ttf",                              make_tuple(resources_ttf_fonts__OpenSans_LightItalic_ttf, resources_ttf_fonts__OpenSans_LightItalic_ttf_len, "application/octet-stream")),
    make_pair("/fonts/OpenSans-Light.ttf",                                    make_tuple(resources_ttf_fonts__OpenSans_Light_ttf, resources_ttf_fonts__OpenSans_Light_ttf_len, "application/octet-stream")),
    make_pair("/fonts/OpenSans-Regular.ttf",                                  make_tuple(resources_ttf_fonts__OpenSans_Regular_ttf, resources_ttf_fonts__OpenSans_Regular_ttf_len, "application/octet-stream")),
    make_pair("/fonts/OpenSans-SemiboldItalic.ttf",                           make_tuple(resources_ttf_fonts__OpenSans_SemiboldItalic_ttf, resources_ttf_fonts__OpenSans_SemiboldItalic_ttf_len, "application/octet-stream")),
    make_pair("/fonts/OpenSans-Semibold.ttf",                                 make_tuple(resources_ttf_fonts__OpenSans_Semibold_ttf, resources_ttf_fonts__OpenSans_Semibold_ttf_len, "application/octet-stream")),
    make_pair("/font-awesome/fonts/FontAwesome.otf",                          make_tuple(resources_otf_font_awesome__fonts__FontAwesome_otf, resources_otf_font_awesome__fonts__FontAwesome_otf_len, "application/octet-stream")),
    make_pair("/font-awesome/fonts/fontawesome-webfont.eot",                  make_tuple(resources_eot_font_awesome__fonts__fontawesome_webfont_eot, resources_eot_font_awesome__fonts__fontawesome_webfont_eot_len, "application/vnd.ms-fontobject")),
    make_pair("/font-awesome/fonts/fontawesome-webfont.svg",                  make_tuple(resources_svg_font_awesome__fonts__fontawesome_webfont_svg, resources_svg_font_awesome__fonts__fontawesome_webfont_svg_len, "image/svg+xml")),
    make_pair("/font-awesome/fonts/fontawesome-webfont.woff",                 make_tuple(resources_woff_font_awesome__fonts__fontawesome_webfont_woff, resources_woff_font_awesome__fonts__fontawesome_webfont_woff_len, "application/font-woff")),
    make_pair("/font-awesome/fonts/fontawesome-webfont.woff2",                make_tuple(resources_woff2_font_awesome__fonts__fontawesome_webfont_woff2, resources_woff2_font_awesome__fonts__fontawesome_webfont_woff2_len, "application/font-woff2")),
    make_pair("/font-awesome/fonts/fontawesome-webfont.ttf",                  make_tuple(resources_ttf_font_awesome__fonts__fontawesome_webfont_ttf, resources_ttf_font_awesome__fonts__fontawesome_webfont_ttf_len, "application/octet-stream")),
    make_pair("/font-ext/fonts/ExtJS.ttf",                                    make_tuple(resources_ttf_font_ext__fonts__ExtJS_ttf, resources_ttf_font_ext__fonts__ExtJS_ttf_len, "application/octet-stream")),
    make_pair("/font-ext/fonts/ExtJS.woff",                                   make_tuple(resources_woff_font_ext__fonts__ExtJS_woff, resources_woff_font_ext__fonts__ExtJS_woff_len, "application/font-woff")),
    make_pair("/font-ext/fonts/ExtJS.svg",                                    make_tuple(resources_svg_font_ext__fonts__ExtJS_svg, resources_svg_font_ext__fonts__ExtJS_svg_len, "image/svg+xml")),
    make_pair("/font-ext/fonts/ExtJS.eot",                                    make_tuple(resources_eot_font_ext__fonts__ExtJS_eot, resources_eot_font_ext__fonts__ExtJS_eot_len, "application/vnd.ms-fontobject")),
};

unsigned int extfs_items_len = 28;

bool get_file(const string& path, vector<unsigned char>& data, string& mimeType, bool& isBinary)
{
    for(unsigned int i = 0; i < extfs_items_len; i++)
    {
        if (path.size() < extfs_items[i].first.size())
            continue;

        if (strstr(path.c_str(), extfs_items[i].first.c_str()) != NULL)
        {
            data.assign(get<0>(extfs_items[i].second), get<0>(extfs_items[i].second) + get<1>(extfs_items[i].second));
            mimeType = get<2>(extfs_items[i].second);
            isBinary = strstr(mimeType.c_str(), "text/") == NULL;
            return true;
        }
    }

    return false;
}
