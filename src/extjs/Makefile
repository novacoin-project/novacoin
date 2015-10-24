OBJS = obj/ext-all-debug.js.o obj/ux-debug.js.o \
       obj/classic__locale__locale-ru-debug.js.o \
       obj/classic__locale__locale-ukr-debug.js.o \
       obj/classic__locale__locale-en-debug.js.o \
       obj/ux__ux-all-debug.css.o \
       obj/classic__theme-triton__resources__theme-triton-all-debug_1.css.o \
       obj/classic__theme-triton__resources__theme-triton-all-debug_2.css.o \
       obj/fonts__OpenSans-BoldItalic.ttf.o \
       obj/fonts__OpenSans-Bold.ttf.o \
       obj/fonts__OpenSans-ExtraBoldItalic.ttf.o \
       obj/fonts__OpenSans-ExtraBold.ttf.o \
       obj/fonts__OpenSans-Italic.ttf.o \
       obj/fonts__OpenSans-LightItalic.ttf.o \
       obj/fonts__OpenSans-Light.ttf.o \
       obj/fonts__OpenSans-Regular.ttf.o \
       obj/fonts__OpenSans-SemiboldItalic.ttf.o \
       obj/fonts__OpenSans-Semibold.ttf.o \
       obj/font-awesome__fonts__FontAwesome.otf.o \
       obj/font-awesome__fonts__fontawesome-webfont.eot.o \
       obj/font-awesome__fonts__fontawesome-webfont.svg.o \
       obj/font-awesome__fonts__fontawesome-webfont.ttf.o \
       obj/font-awesome__fonts__fontawesome-webfont.woff.o \
       obj/font-awesome__fonts__fontawesome-webfont.woff2.o \
       obj/font-ext__fonts__ExtJS.eot.o \
       obj/font-ext__fonts__ExtJS.svg.o \
       obj/font-ext__fonts__ExtJS.ttf.o \
       obj/font-ext__fonts__ExtJS.woff.o \
       obj/index.o


obj/index.o: index.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

obj/ext-all-debug.js.o: ext-all-debug.js
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

obj/ux-debug.js.o: ux-debug.js
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

obj/%.js.o: resources/js/%.js
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

obj/%.css.o: resources/css/%.css
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

obj/%.ttf.o: resources/ttf/%.ttf
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

obj/%.eot.o: resources/eot/%.eot
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

obj/%.svg.o: resources/svg/%.svg.o
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

obj/%.woff.o: resources/woff/%.woff
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

obj/%.woff2.o: resources/woff2/%.woff2
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

obj/%.otf.o: resources/otf/%.otf
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

obj/%.svg.o: resources/svg/%.svg
	xxd -i $< | $(CXX) -x c++ -c $(CXXFLAGS) -o $@ -

all: $(OBJS:obj/%=obj/%)

clean:
	-rm -f obj/*.o
