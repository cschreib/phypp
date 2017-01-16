#ifndef PHYPP_IO_ASTRO_WCS_HPP
#define PHYPP_IO_ASTRO_WCS_HPP

#ifndef NO_WCSLIB
#include <wcslib/wcshdr.h>
#include <wcslib/wcserr.h>
#endif

#include "phypp/core/error.hpp"
#include "phypp/io/fits.hpp"
#include "phypp/astro/astro.hpp"

namespace phypp {
namespace astro {
    struct make_wcs_header_params {
        // The pixel size in arcsec
        double pixel_scale = dnan;
        // The reference position
        double sky_ref_ra = dnan, sky_ref_dec = dnan;
        // The pixel corresponding to the reference position
        double pixel_ref_x = dnan, pixel_ref_y = dnan;
        // The number of pixels in X and Y axis
        uint_t dims_x = npos, dims_y = npos;
    };

    // Add WCS data to a FITS header, computed from a set of simple parameters.
    inline bool make_wcs_header(const make_wcs_header_params& params, fits::header& hdr) {
        if (hdr.empty()) {
            hdr = "END" + std::string(77, ' ');
        }

        if (is_finite(params.pixel_scale)) {
            if (!fits::setkey(hdr, "CDELT1", -params.pixel_scale/3600.0)) {
                error("make_wcs_header: could not set keyword 'CDELT1' to '",
                    -params.pixel_scale, "'");
                return false;
            }
            if (!fits::setkey(hdr, "CDELT2", params.pixel_scale/3600.0)) {
                error("make_wcs_header: could not set keyword 'CDELT2' to '",
                    params.pixel_scale, "'");
                return false;
            }
            if (!fits::setkey(hdr, "CTYPE1", "'RA---TAN'")) {
                error("make_wcs_header: could not set keyword 'CTYPE1' to 'RA---TAN'");
                return false;
            }
            if (!fits::setkey(hdr, "CTYPE2", "'DEC--TAN'")) {
                error("make_wcs_header: could not set keyword 'CTYPE2' to 'DEC--TAN'");
                return false;
            }
            if (!fits::setkey(hdr, "EQUINOX", 2000.0)) {
                error("make_wcs_header: could not set keyword 'EQUINOX' to '",
                    2000.0, "'");
                return false;
            }
        }

        if (is_finite(params.pixel_ref_x) && is_finite(params.pixel_ref_y)) {
            if (!fits::setkey(hdr, "CRPIX1", params.pixel_ref_x)) {
                error("make_wcs_header: could not set keyword 'CRPIX1' to '",
                    params.pixel_ref_x, "'");
                return false;
            }
            if (!fits::setkey(hdr, "CRPIX2", params.pixel_ref_y)) {
                error("make_wcs_header: could not set keyword 'CRPIX2' to '",
                    params.pixel_ref_y, "'");
                return false;
            }
        }

        if (is_finite(params.sky_ref_ra) && is_finite(params.sky_ref_dec)) {
            if (!fits::setkey(hdr, "CRVAL1", params.sky_ref_ra)) {
                error("make_wcs_header: could not set keyword 'CRVAL1' to '",
                    params.sky_ref_ra, "'");
                return false;
            }
            if (!fits::setkey(hdr, "CRVAL2", params.sky_ref_dec)) {
                error("make_wcs_header: could not set keyword 'CRVAL2' to '",
                    params.sky_ref_dec, "'");
                return false;
            }
        }

        if (params.dims_x != npos && params.dims_y != npos) {
            if (!fits::setkey(hdr, "NAXES", 2u)) {
                error("make_wcs_header: could not set keyword 'NAXES' to '", 2u, "'");
                return false;
            }
            if (!fits::setkey(hdr, "NAXIS1", params.dims_x)) {
                error("make_wcs_header: could not set keyword 'NAXIS1' to '",
                    params.dims_x, "'");
                return false;
            }
            if (!fits::setkey(hdr, "NAXIS2", params.dims_y)) {
                error("make_wcs_header: could not set keyword 'NAXIS2' to '",
                    params.dims_y, "'");
                return false;
            }
            if (!fits::setkey(hdr, "META_0", 2u)) {
                error("make_wcs_header: could not set keyword 'META_0' to '", 2u, "'");
                return false;
            }
            if (!fits::setkey(hdr, "META_1", params.dims_x)) {
                error("make_wcs_header: could not set keyword 'META_1' to '",
                    params.dims_x, "'");
                return false;
            }
            if (!fits::setkey(hdr, "META_2", params.dims_y)) {
                error("make_wcs_header: could not set keyword 'META_2' to '",
                    params.dims_y, "'");
                return false;
            }
        }

        return true;
    }

    // Add WCS data to a FITS header, computed from a set of simple parameters.
    // Format: {"pixel_scale:0.06", "sky_ref:-3.56985,52.6456", ...}
    // Parameters:
    //  - pixel_scale [float]: the pixel size in arcsec
    //  - sky_ref [float,float]: the reference position
    //  - pixel_ref [float,float]: the pixel corresponding to the reference position
    //  - dims [uint,uint]: number of pixels in X and Y axis
    inline bool make_wcs_header(const vec1s& string_params, fits::header& hdr) {
        make_wcs_header_params params;

        for (auto& p : string_params) {
            vec1s spl = split(p, ":");

            if (spl.size() != 2) {
                error("make_wcs_header: parameter '", p, "' is ill formed");
                return false;
            }

            spl[0] = trim(tolower(spl[0]));

            if (spl[0] == "pixel_scale") {
                if (!from_string(spl[1], params.pixel_scale)) {
                    error("make_wcs_header: could not read pixel scale '",
                        spl[1], "' as double");
                    return false;
                }
            } else if (spl[0] == "pixel_ref") {
                vec1s tspl = split(spl[1], ",");
                if (tspl.size() != 2) {
                    error("make_wcs_header: ill formed 'pixel_ref' parameter: '", spl[0], "'");
                    note("make_wcs_header: expecting two comma separated coordinates of "
                        "reference pixel");
                    return false;
                }
                if (!from_string(tspl[0], params.pixel_ref_x)) {
                    error("make_wcs_header: could not read X pixel reference '",
                        tspl[0], "' as double");
                    return false;
                }
                if (!from_string(tspl[1], params.pixel_ref_y)) {
                    error("make_wcs_header: could not read Y pixel reference '",
                        tspl[1], "' as double");
                    return false;
                }
            } else if (spl[0] == "sky_ref") {
                vec1s tspl = split(spl[1], ",");
                if (tspl.size() != 2) {
                    error("make_wcs_header: ill formed 'sky_ref' parameter: '", spl[0], "'");
                    note("make_wcs_header: expecting two comma separated coordinates of "
                        "reference sky position");
                    return false;
                }
                if (!from_string(tspl[0], params.sky_ref_ra)) {
                    error("make_wcs_header: could not read RA sky position reference '",
                        tspl[0], "' as double");
                    return false;
                }
                if (!from_string(tspl[1], params.sky_ref_dec)) {
                    error("make_wcs_header: could not read Dec sky position reference '",
                        tspl[1], "' as double");
                    return false;
                }
            } else if (spl[0] == "dims") {
                vec1s tspl = split(spl[1], ",");
                if (tspl.size() != 2) {
                    error("make_wcs_header: ill formed 'dims' parameter: '", spl[0], "'");
                    note("make_wcs_header: expecting two comma separated number of pixels");
                    return false;
                }
                if (!from_string(tspl[0], params.dims_x)) {
                    error("make_wcs_header: could not read number of pixels in first axis '",
                        tspl[0], "' as unsigned integer");
                    return false;
                }
                if (!from_string(tspl[1], params.dims_y)) {
                    error("make_wcs_header: could not read number of pixels in second axis '",
                        tspl[1], "' as unsigned integer");
                    return false;
                }
            } else {
                error("make_wcs_header: unknown parameter '", spl[0], "'");
                return false;
            }
        }

        return make_wcs_header(params, hdr);
    }

    inline fits::header filter_wcs(const fits::header& hdr) {
        // List of keywords taken from 'cphead' (WCSTools)
        vec1s keywords = {"RA", "DEC", "EPOCH", "EQUINOX", "RADECSYS", "SECPIX", "IMWCS",
            "CD1_1", "CD1_2", "CD2_1", "CD2_2", "PC1_1", "PC1_2", "PC2_1", "PC2_2",
            "PC001001", "PC001002", "PC002001", "PC002002", "LATPOLE", "LONPOLE",
            "SECPIX", "CTYPE", "CRVAL", "CDELT", "CRPIX", "CROTA",
            "CUNIT", "CO1_", "CO2_", "PROJP", "PV1_", "PV2_", "END"};

        vec1s okeys = cut(hdr, 80);
        vec1s nkeys;
        for (auto& k : okeys) {
            for (auto& wk : keywords) {
                if (start_with(k, wk)) {
                    nkeys.push_back(k);
                }
            }
        }

        return collapse(nkeys);
    }
}

namespace impl {
namespace wcs_impl {
    struct header_keyword {
        std::string key, value, comment;
        bool novalue = true;
    };

    inline std::vector<header_keyword> parse_header(const fits::header& hdr) {
        std::vector<header_keyword> keys;
        vec1s ckeys = cut(hdr, 80);
        keys.resize(ckeys.size());
        for (uint_t i : range(ckeys)) {
            if (ckeys[i].find_first_of("HISTORY ") == 0) {
                keys[i].key = trim(ckeys[i]);
            } else {
                auto p = ckeys[i].find_first_of("=/");
                if (p == std::string::npos) {
                    keys[i].key = trim(ckeys[i]);
                } else if (ckeys[i][p] == '/') {
                    keys[i].key = trim(ckeys[i].substr(0, p));
                    keys[i].comment = trim(ckeys[i].substr(p));
                } else if (ckeys[i][p] == '=') {
                    keys[i].novalue = false;
                    keys[i].key = trim(ckeys[i].substr(0, p));

                    std::string right = trim(ckeys[i].substr(p+1));
                    if (!right.empty()) {
                        if (right[0] == '\'') {
                            auto p2 = right.find_first_of('\'', 1);
                            if (p2 != std::string::npos) {
                                ++p2;

                                keys[i].value = trim(right.substr(0, p2));
                                keys[i].comment = trim(right.substr(p2));
                            } else {
                                keys[i].value = trim(right);
                            }
                        } else {
                            uint_t p2 = right.find_first_of('/');
                            if (p2 != std::string::npos) {
                                keys[i].value = trim(right.substr(0, p2));
                                keys[i].comment = trim(right.substr(p2));
                            } else {
                                keys[i].value = trim(right);
                            }
                        }
                    }
                }
            }
        }

        return keys;
    }

    inline fits::header serialize_header(const std::vector<header_keyword>& keys) {
        fits::header hdr;
        hdr.reserve(80*keys.size());
        for (auto& k : keys) {
            std::string entry;
            if (!k.novalue) {
                if (start_with(k.key, "HIERARCH ")) {
                    entry = k.key+" = "+k.value;
                } else {
                    std::string val;
                    if (k.value[0] == '\'') {
                        val = align_left(k.value, 20, ' ');
                    } else {
                        val = align_right(k.value, 20, ' ');
                    }
                    entry = align_left(k.key, 8, ' ')+"= "+val;
                }
            } else {
                entry = align_left(k.key, 30, ' ');
            }

            if (!k.comment.empty()) {
                entry += " "+k.comment;
            }

            if (entry.size() > 80) {
                entry = entry.substr(0, 80);
            } else if (entry.size() < 80) {
                entry += std::string(80-entry.size(), ' ');
            }

            hdr += entry;
        }

        return hdr;
    }

    inline void cure_header(fits::header& hdr) {
        auto keys = parse_header(hdr);

        for (auto& k : keys) {
            if (start_with(k.key, "CUNIT")) {
                // Fix non-standard units
                std::string v = trim(k.value, "' ");
                if (v == "micron" || v == "microns") {
                    k.value = "'um'";
                } else if (v == "degree" || v == "degrees") {
                    k.value = "'deg'";
                }
            }
        }

        hdr = serialize_header(keys);
    }
}
}

namespace astro {

    enum class axis_unit {
        native,
        wcslib_default,

        wave_m,
        wave_cm,
        wave_mm,
        wave_um,
        wave_nm,
        wave_Angstrom,

        freq_Hz,
        freq_kHz,
        freq_MHz,
        freq_GHz,
        freq_THz,

        sky_deg,
        sky_rad
    };

    enum class axis_type {
        spatial, wave, freq, unknown
    };

    inline std::string axis_type_string(axis_type type) {
        switch (type) {
            case axis_type::spatial : return "spatial";
            case axis_type::wave :    return "wavelength";
            case axis_type::freq :    return "frequency";
            case axis_type::unknown : return "unknown";
        }
    }

#ifndef NO_WCSLIB
    // Extract astrometry from a FITS image header
    struct wcs {
        wcsprm* w = nullptr;
        int nwcs  = 0;

        vec1u dims;
        vec1b has_unit;
        vec<1,axis_type> type;

        uint_t ra_axis = 1, dec_axis = 0;
        uint_t x_axis = 1, y_axis = 0;

        explicit wcs(uint_t naxis = 2) : w(new wcsprm), nwcs(1) {
            w->flag = -1;
            wcsini(true, naxis, w);

            dims = replicate(0, naxis);
            has_unit = replicate(false, naxis);
            type = replicate(axis_type::unknown, naxis);
        }

        explicit wcs(fits::header hdr) {
            // Cure header for ingestion by WCSlib
            impl::wcs_impl::cure_header(hdr);

            // Enable error reporting
            wcserr_enable(1);

            // Feed the header to WCSLib to extract the astrometric parameters
            int nreject = 0;
            int success = wcspih(
                const_cast<char*>(hdr.c_str()), hdr.size()/80 + 1,
                WCSHDR_all, 0, &nreject, &nwcs, &w
            );

            if ((success != 0 || nwcs == 0) && w) {
                wcsvfree(&nwcs, &w);
                w = nullptr;
            }

            if (w) {
                // Get dimensions from the FITS header
                uint_t naxis = axis_count();
                dims.resize(naxis);
                for (uint_t i : range(dims)) {
                    uint_t dim = npos;
                    if (fits::getkey(hdr, "NAXIS"+strn(i+1), dim)) {
                        dims[naxis-1-i] = dim;
                    }
                }

                // Check if axes have units (WCSlib will be silent about that)
                has_unit.resize(naxis);
                for (uint_t i : range(dims)) {
                    has_unit[naxis-1-i] = (trim(w->cunit[i]) != "");
                }

                // Get types of axis
                type = replicate(axis_type::unknown, naxis);
                for (uint_t i : range(dims)) {
                    std::string tmp = split(w->ctype[i], "-")[0];
                    if (tmp == "RA") {
                        type[naxis-1-i] = axis_type::spatial;
                    } else if (tmp == "DEC") {
                        type[naxis-1-i] = axis_type::spatial;
                    } else if (tmp == "WAVE") {
                        type[naxis-1-i] = axis_type::wave;
                    } else if (tmp == "FREQ") {
                        type[naxis-1-i] = axis_type::freq;
                    }
                }

                // Identify RA and Dec axis
                uint_t tx = find_axis("RA");
                uint_t ty = find_axis("DEC");
                if (tx != npos && tx != npos) {
                    ra_axis = x_axis = tx;
                    dec_axis = y_axis = ty;

                    // Y is by definition the first axis, so swap them if
                    // the input file has DEC/RA instead of RA/DEC
                    if (x_axis < y_axis) std::swap(x_axis, y_axis);
                }

                // Try a dummy coordinate conversion to see if everything is recognized
                vec1d map = replicate(0.0, w->naxis);
                vec1d world(w->naxis);
                vec1d itmp(w->naxis);
                double phi, theta;
                int status = 0;

                int ret = wcsp2s(w, 1, w->naxis, map.data.data(), itmp.data.data(), &phi, &theta,
                    world.data.data(), &status);

                if (ret != 0) {
                    wcserr_prt(w->err, "error: ");
                    wcsvfree(&nwcs, &w);
                    w = nullptr;
                }
            }
        }

        uint_t axis_count() const {
            return (w ? w->naxis : 0);
        }

        uint_t find_axis(const std::string& name) const {
            for (uint_t i : range(axis_count())) {
                std::string ctype = split(w->ctype[i], "-")[0];
                if (ctype == toupper(name)) {
                    return w->naxis-1 - i;
                }
            }

            return npos;
        }

        bool valid_unit(uint_t axis, axis_unit unit, std::string& why) const {
            if (axis >= axis_count()) {
                why = "axis "+strn(axis)+" does not exist";
                return false;
            }

            if (has_unit[axis]) {
                if (unit == axis_unit::native) {
                    why = "requesting native units for an axis with specified units it not implemented yet!";
                    return false;
                }
            } else {
                if (unit != axis_unit::native) {
                    why = "axis "+strn(axis)+" has no CUNIT keyword";
                    return false;
                }
            }

            axis_type unit_type = axis_type::unknown;
            switch (unit) {
                case astro::axis_unit::native:         unit_type = axis_type::unknown; break;
                case astro::axis_unit::wcslib_default: unit_type = axis_type::unknown; break;

                case astro::axis_unit::wave_m:         unit_type = axis_type::wave; break;
                case astro::axis_unit::wave_cm:        unit_type = axis_type::wave; break;
                case astro::axis_unit::wave_mm:        unit_type = axis_type::wave; break;
                case astro::axis_unit::wave_um:        unit_type = axis_type::wave; break;
                case astro::axis_unit::wave_nm:        unit_type = axis_type::wave; break;
                case astro::axis_unit::wave_Angstrom:  unit_type = axis_type::wave; break;

                case astro::axis_unit::freq_Hz:        unit_type = axis_type::freq; break;
                case astro::axis_unit::freq_kHz:       unit_type = axis_type::freq; break;
                case astro::axis_unit::freq_MHz:       unit_type = axis_type::freq; break;
                case astro::axis_unit::freq_GHz:       unit_type = axis_type::freq; break;
                case astro::axis_unit::freq_THz:       unit_type = axis_type::freq; break;

                case astro::axis_unit::sky_deg:        unit_type = axis_type::spatial; break;
                case astro::axis_unit::sky_rad:        unit_type = axis_type::spatial; break;
            }

            if (type[axis] != axis_type::unknown && unit_type != axis_type::unknown &&
                type[axis] != unit_type) {
                why = "wrong type for axis "+strn(axis)+" (expected "+axis_type_string(type[axis])+
                    ", got "+axis_type_string(unit_type);
                return false;
            }

            return true;
        }

        wcs(const wcs&) = delete;
        wcs& operator = (const wcs&) = delete;

        wcs(wcs&& tw) {
            std::swap(w, tw.w);
            std::swap(nwcs, tw.nwcs);
            std::swap(dims, tw.dims);
            std::swap(has_unit, tw.has_unit);
            std::swap(type, tw.type);
            std::swap(ra_axis, tw.ra_axis);
            std::swap(dec_axis, tw.dec_axis);
            std::swap(x_axis, tw.x_axis);
            std::swap(y_axis, tw.y_axis);
        }

        wcs& operator = (wcs&& tw) {
            if (w) {
                wcsvfree(&nwcs, &w);
            }

            w = tw.w; tw.w = nullptr;
            nwcs = tw.nwcs; tw.nwcs = 0;
            dims = tw.dims; tw.dims.clear();
            has_unit = tw.has_unit; tw.has_unit.clear();
            type = tw.type; tw.type.clear();
            ra_axis = tw.ra_axis;
            dec_axis = tw.dec_axis;
            x_axis = tw.x_axis;
            y_axis = tw.y_axis;

            return *this;
        }

        bool is_valid() const {
            return w != nullptr;
        }

        ~wcs() {
            if (w) {
                wcsvfree(&nwcs, &w);
                w = nullptr;
            }
        }
    };
#else
    struct wcs {
        template<typename T, typename ... Args>
        wcs(Args&&...) {
            static_assert(!std::is_same<T,T>::value, "WCS support is is disabled, "
                "please enable the WCSLib library to use this function");
        }
    };
#endif

    template<typename Dummy = void>
    astro::wcs extast(const fits::header& hdr) {
#ifdef NO_WCSLIB
        static_assert(!std::is_same<Dummy,Dummy>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else
        return astro::wcs(hdr);
#endif
    }

}

namespace impl {
    namespace wcs_impl {
        inline vec2d world2pix(const astro::wcs& w, const vec2d& world) {
            vec2d pix(world.dims);

            uint_t npt = world.dims[0];
            uint_t naxis = world.dims[1];

            std::vector<double> phi(npt), theta(npt);
            std::vector<double> itmp(naxis*npt);
            std::vector<int>    stat(npt);

            int status = wcss2p(w.w, npt, naxis, world.data.data(), phi.data(), theta.data(),
                itmp.data(), pix.data.data(), stat.data());

            if (status != 0) {
                wcserr_prt(w.w->err, "error: ");
            }

            phypp_check(status == 0, "error in WCS conversion");

            return pix;
        }

        inline vec2d pix2world(const astro::wcs& w, const vec2d& pix) {
            vec2d world(pix.dims);

            uint_t npt = pix.dims[0];
            uint_t naxis = pix.dims[1];

            std::vector<double> phi(npt), theta(npt);
            std::vector<double> itmp(naxis*npt);
            std::vector<int>    stat(npt);

            int status = wcsp2s(w.w, npt, naxis, pix.data.data(), itmp.data(),
                phi.data(), theta.data(), world.data.data(), stat.data());

            if (status != 0) {
                wcserr_prt(w.w->err, "error: ");
            }

            phypp_check(status == 0, "error in WCS conversion");

            return world;
        }
    }
}

namespace astro {

    template<std::size_t D = 1, typename T = double, typename U = double, typename V, typename W>
    void ad2xy(const astro::wcs& w, const vec<D,T>& ra, const vec<D,U>& dec,
        vec<D,V>& x, vec<D,W>& y) {
#ifdef NO_WCSLIB
        static_assert(!std::is_same<T,T>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else

        phypp_check(w.is_valid(), "invalid WCS data");
        phypp_check(ra.dims == dec.dims, "RA and Dec arrays do not match sizes ("+
            strn(ra.dims)+" vs "+strn(dec.dims)+")");

        uint_t npt = ra.size();
        if (npt == 0) {
            x.clear(); y.clear();
            return;
        }

        uint_t naxis = w.axis_count();
        vec2d world(npt, naxis);
        for (uint_t i : range(npt)) {
            world.safe(i,naxis-1-w.ra_axis) = ra.safe[i];
            world.safe(i,naxis-1-w.dec_axis) = dec.safe[i];
        }

        vec2d pix = impl::wcs_impl::world2pix(w, world);

        x.resize(ra.dims);
        y.resize(ra.dims);

        for (uint_t i : range(npt)) {
            x.safe[i] = pix.safe(i,naxis-1-w.x_axis);
            y.safe[i] = pix.safe(i,naxis-1-w.y_axis);
        }
#endif
    }

    template<std::size_t D = 1, typename T = double, typename U = double, typename V, typename W>
    void xy2ad(const astro::wcs& w, const vec<D,T>& x, const vec<D,U>& y,
        vec<D,V>& ra, vec<D,W>& dec) {
#ifdef NO_WCSLIB
        static_assert(!std::is_same<T,T>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else

        phypp_check(w.is_valid(), "invalid WCS data");
        phypp_check(x.dims == y.dims, "x and y arrays do not match sizes ("+
            strn(x.dims)+" vs "+strn(y.dims)+")");

        uint_t npt = x.size();
        if (npt == 0) {
            ra.clear(); dec.clear();
            return;
        }

        uint_t naxis = w.axis_count();
        vec2d pix(npt, naxis);
        for (uint_t i : range(npt)) {
            pix.safe(i,naxis-1-w.x_axis) = x.safe[i];
            pix.safe(i,naxis-1-w.y_axis) = y.safe[i];
        }

        vec2d world = impl::wcs_impl::pix2world(w, pix);

        ra.resize(x.dims);
        dec.resize(x.dims);

        for (uint_t i : range(npt)) {
            ra.safe[i] = world.safe(i,naxis-1-w.ra_axis);
            dec.safe[i] = world.safe(i,naxis-1-w.dec_axis);
        }
#endif
    }

    template<typename T = double, typename U = double, typename V, typename W,
        typename enable = typename std::enable_if<!meta::is_vec<T>::value &&
            !meta::is_vec<U>::value && !meta::is_vec<V>::value && !meta::is_vec<W>::value>::type>
    void ad2xy(const astro::wcs& w, const T& ra, const U& dec, V& x, W& y) {
#ifdef NO_WCSLIB
        static_assert(!std::is_same<T,T>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else
        vec<1,T> tra  = replicate(ra,  1);
        vec<1,T> tdec = replicate(dec, 1);
        vec<1,V> tx;
        vec<1,W> ty;

        ad2xy(w, tra, tdec, tx, ty);

        x = tx.safe[0];
        y = ty.safe[0];
#endif
    }

    template<typename T = double, typename U = double, typename V, typename W,
        typename enable = typename std::enable_if<!meta::is_vec<T>::value &&
            !meta::is_vec<U>::value && !meta::is_vec<V>::value && !meta::is_vec<W>::value>::type>
    void xy2ad(const astro::wcs& w, const T& x, const U& y, V& ra, W& dec) {
#ifdef NO_WCSLIB
        static_assert(!std::is_same<T,T>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else
        vec<1,T> tx = replicate(x, 1);
        vec<1,T> ty = replicate(y, 1);
        vec<1,V> tra;
        vec<1,W> tdec;

        xy2ad(w, tx, ty, tra, tdec);

        ra = tra.safe[0];
        dec = tdec.safe[0];
#endif
    }

    // Obtain the pixel size of a given image in arsec/pixel.
    // Will fail (return false) if no WCS information is present in the image.
    template<typename Dummy = void>
    bool get_pixel_size(const astro::wcs& wcs, double& aspix) {
#ifdef NO_WCSLIB
        static_assert(!std::is_same<Dummy,Dummy>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else
        if (!wcs.is_valid()) {
            return false;
        }

        // Convert radius to number of pixels
        vec1d r, d;
        astro::xy2ad(wcs, {0, 1}, {0, 0}, r, d);
        aspix = angdist(r.safe[0], d.safe[0], r.safe[1], d.safe[1]);

        return true;
#endif
    }

    // Obtain the pixel size of a given image in arsec/pixel.
    // Will fail (return false) if no WCS information is present in the image.
    template<typename Dummy = void>
    bool get_pixel_size(const std::string& file, double& aspix) {
#ifdef NO_WCSLIB
        static_assert(!std::is_same<Dummy,Dummy>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else
        if (end_with(file, ".sectfits")) {
            vec1s sects = fits::read_sectfits(file);
            return get_pixel_size(sects[0], aspix);
        } else {
            fits::header hdr = fits::read_header(file);
            auto wcs = astro::wcs(hdr);
            if (!get_pixel_size(wcs, aspix)) {
                warning("could not extract WCS information");
                note("parsing '", file, "'");
                return false;
            }

            return true;
        }
#endif
    }

}

namespace impl {
    namespace wcs_impl {
        inline double conv_si2unit(astro::axis_unit unit) {
            switch (unit) {
                case astro::axis_unit::native:         return 1.0;

                // WCSlib returns data in SI units or degrees
                case astro::axis_unit::wcslib_default: return 1.0;

                case astro::axis_unit::wave_m:         return 1.0;
                case astro::axis_unit::wave_cm:        return 1e2;
                case astro::axis_unit::wave_mm:        return 1e3;
                case astro::axis_unit::wave_um:        return 1e6;
                case astro::axis_unit::wave_nm:        return 1e9;
                case astro::axis_unit::wave_Angstrom:  return 1e10;

                case astro::axis_unit::freq_Hz:        return 1.0;
                case astro::axis_unit::freq_kHz:       return 1e-3;
                case astro::axis_unit::freq_MHz:       return 1e-6;
                case astro::axis_unit::freq_GHz:       return 1e-9;
                case astro::axis_unit::freq_THz:       return 1e-12;

                case astro::axis_unit::sky_deg:        return 1.0;
                case astro::axis_unit::sky_rad:        return dpi/180.0;
            }
        }

        template<std::size_t D, typename T>
        void si2unit(vec<D,T>& data, astro::axis_unit unit) {
            double conv = conv_si2unit(unit);
            if (conv != 1.0) {
                data *= conv;
            }
        }

        template<std::size_t D, typename T>
        void unit2si(vec<D,T>& data, astro::axis_unit unit) {
            double conv = conv_si2unit(unit);
            if (conv != 1.0) {
                data /= conv;
            }
        }
    }
}

namespace astro {

    template<std::size_t D = 1, typename T = double, typename U>
    void x2w(const astro::wcs& wcs, uint_t axis, const vec<D,T>& x, vec<D,U>& w,
        axis_unit unit = axis_unit::wcslib_default) {

#ifdef NO_WCSLIB
        static_assert(!std::is_same<T,T>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else
        uint_t naxis = wcs.axis_count();

        phypp_check(wcs.is_valid(), "invalid WCS data");
        phypp_check(axis < naxis, "trying to use an axis that does not exist (",
            axis, " vs ", naxis, ")");

        std::string why;
        bool vunit = wcs.valid_unit(axis, unit, why);
        phypp_check(vunit, why);

        uint_t npix = x.size();

        vec2d pix(npix, naxis);
        pix.safe(_,naxis-1-axis) = x;
        for (uint_t i : range(naxis)) {
            if (i == axis) continue;
            pix.safe(_,naxis-1-i) = wcs.w->crpix[naxis-1-i];
        }

        vec2d world = impl::wcs_impl::pix2world(wcs, pix);
        w = world.safe(_,naxis-1-axis);
        impl::wcs_impl::si2unit(w, unit);
#endif
    }

    template<std::size_t D = 1, typename T = double, typename U>
    void w2x(const astro::wcs& wcs, uint_t axis, const vec<D,T>& w, vec<D,U>& x,
        axis_unit unit = axis_unit::wcslib_default) {

#ifdef NO_WCSLIB
        static_assert(!std::is_same<T,T>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else
        uint_t naxis = wcs.axis_count();

        phypp_check(wcs.is_valid(), "invalid WCS data");
        phypp_check(axis < naxis, "trying to use an axis that does not exist (",
            axis, " vs ", naxis, ")");

        std::string why;
        bool vunit = wcs.valid_unit(axis, unit, why);
        phypp_check(vunit, why);

        uint_t npix = w.size();

        vec1d tw = w;
        impl::wcs_impl::unit2si(tw, unit);

        vec2d world(npix, naxis);
        world.safe(_,naxis-1-axis) = tw;

        for (uint_t i : range(naxis)) {
            if (i == axis) continue;
            world.safe(_,naxis-1-i) = wcs.w->crval[naxis-1-i];
        }

        vec2d pix = impl::wcs_impl::world2pix(wcs, world);
        x = pix.safe(_,naxis-1-axis);
#endif
    }

    template<typename Dummy = void>
    void x2w(const astro::wcs& wcs, uint_t axis, double x, double& w,
        axis_unit unit = axis_unit::wcslib_default) {

#ifdef NO_WCSLIB
        static_assert(!std::is_same<Dummy,Dummy>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else

        vec1d tx = replicate(x, 1);
        vec1d tw;

        x2w(wcs, axis, tx, tw, unit);

        w = tw.safe[0];
#endif
    }

    template<typename Dummy = void>
    void w2x(const astro::wcs& wcs, uint_t axis, double w, double& x,
        axis_unit unit = axis_unit::wcslib_default) {

#ifdef NO_WCSLIB
        static_assert(!std::is_same<Dummy,Dummy>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else

        vec1d tw = replicate(w, 1);
        vec1d tx;

        w2x(wcs, axis, tw, tx, unit);

        x = tx.safe[0];
#endif
    }

    // Obtain the pixel size of a given image in arsec/pixel.
    // Will fail (return false) if no WCS information is present in the image.
    template<typename Type = double>
    vec<1,Type> build_axis(const astro::wcs& wcs, uint_t axis, axis_unit unit = axis_unit::wcslib_default) {
        vec<1,Type> ret;

#ifdef NO_WCSLIB
        static_assert(!std::is_same<Type,Type>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else
        uint_t naxis = wcs.axis_count();

        phypp_check(wcs.is_valid(), "invalid WCS data");
        phypp_check(axis < naxis, "trying to use an axis that does not exist (",
            axis, " vs ", naxis, ")");

        std::string why;
        bool vunit = wcs.valid_unit(axis, unit, why);
        phypp_check(vunit, why);

        uint_t npix = wcs.dims[axis];

        vec2d pix(npix, naxis);
        pix.safe(_,naxis-1-axis) = dindgen(npix)+1;
        for (uint_t i : range(naxis)) {
            if (i == axis) continue;
            pix.safe(_,naxis-1-i) = wcs.w->crpix[naxis-1-i];
        }

        vec2d world = impl::wcs_impl::pix2world(wcs, pix);

        ret = world.safe(_,naxis-1-axis);
        impl::wcs_impl::si2unit(ret, unit);
#endif

        return ret;
    }
}

namespace impl {
    namespace wcs_impl {
        // Convenience functions
        inline double regrid_drizzle_getmin(const vec1d& v) {
            double tmp = floor(min(v));
            return tmp > 0.0 ? tmp : 0.0;
        };
        inline double regrid_drizzle_getmax(const vec1d& v, double n) {
            double tmp = ceil(max(v));
            return tmp > n ? n : tmp;
        };

        // Function to find if a point lies on the right side of a polygon's edge
        // orient: orientation of the polygon
        // cx1, cy1, cx2, cy2: X and Y coordinates of the two nodes of the edge
        // x, y: X and Y coordinates of the point to test
        // return: true if the point is on the right side, false otherwise
        inline bool regrid_drizzle_in_poly_edge(int_t orient,
            double cx1, double cy1, double cx2, double cy2, double x, double y) {
            double cross = (cx2 - cx1)*(y - cy1) - (cy2 - cy1)*(x - cx1);
            return cross*orient < 0;
        };

        // Function to find the position and existence of the intersection of two lines
        // l1x1, l1y1, l1x2, l1y2: X and Y coordinates of two points defining the first line
        // l2x1, l2y1, l2x2, l2y2: X and Y coordinates of two points defining the second line
        // x, y: output X and Y coordinates of the intersection point (if any)
        // return: true if intersection exists, false otherwise
        inline bool regrid_drizzle_segment_intersect(
            double l1x1, double l1y1, double l1x2, double l1y2,
            double l2x1, double l2y1, double l2x2, double l2y2, double& x, double& y) {

            // Find the intersection point
            // http://stackoverflow.com/a/1968345/1565581
            double s1x = l1x2 - l1x1;
            double s1y = l1y2 - l1y1;
            double s2x = l2x2 - l2x1;
            double s2y = l2y2 - l2y1;

            double det = s1x*s2y - s1y*s2x;
            if (abs(det) < 5*std::numeric_limits<double>::epsilon()) {
                // 'det' is zero: the lines are parallel, no intersection
                return false;
            }

            double s12x = l1x1 - l2x1;
            double s12y = l1y1 - l2y1;
            double t = (s2x*s12y - s2y*s12x)/det;
            x = l1x1 + t*s1x;
            y = l1y1 + t*s1y;

            return true;
        };

        inline double polyon_area(const vec1d& x, const vec1d& y) {
            // phypp_check(x.size() == y.size(), "incompatible dimensions between X and Y arrays (",
            //    x.size(), " vs. ", y.size(), ")");

            // Note: the above check is not performed here for performance reasons
            // and because this is an internal function. If you want to use it outside,
            // please uncomment the check

            double area = 0.0;

            if (x.size() < 3) return area;

            uint_t i3 = x.size()-2;
            uint_t i2 = x.size()-1;
            for (uint_t i1 : range(x.size()-2)) {
                area += 0.5*abs(
                    x.safe[i1]*(y.safe[i2]-y.safe[i3]) +
                    x.safe[i2]*(y.safe[i3]-y.safe[i1]) +
                    x.safe[i3]*(y.safe[i1]-y.safe[i2])
                );

                i3 = i2;
                i2 = i1;
            }

            return area;
        }

        template<typename T, typename U>
        bool regrid_drizzle(const vec<2,T>& imgs, const vec1d& xps, const vec1d& yps, U& flx) {
            // Get bounds of this projection
            uint_t ymin = regrid_drizzle_getmin(yps-0.5);
            uint_t xmin = regrid_drizzle_getmin(xps-0.5);
            int_t tymax = regrid_drizzle_getmax(yps+0.5, imgs.dims[0]-1);
            int_t txmax = regrid_drizzle_getmax(xps+0.5, imgs.dims[1]-1);

            if (tymax < 0 || txmax < 0) {
                return false;
            }

            uint_t ymax = tymax, xmax = txmax;

            // Get polygon orientation
            int_t orient = ((xps.safe[1] - xps.safe[0])*(yps.safe[2] - yps.safe[1]) -
                (yps.safe[1] - yps.safe[0])*(xps.safe[2] - xps.safe[1]) > 0) ? 1 : -1;

            // Sum flux from original pixels that fall inside the projection
            bool covered = false;
            for (uint_t ipy = ymin; ipy <= ymax; ++ipy)
            for (uint_t ipx = xmin; ipx <= xmax; ++ipx) {
                // Construct the intersection polygon of the original pixel and the projection
                // https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm
                vec1d cpx = {ipx-0.5, ipx+0.5, ipx+0.5, ipx-0.5};
                vec1d cpy = {ipy-0.5, ipy-0.5, ipy+0.5, ipy+0.5};

                uint_t c2 = xps.size()-1;
                for (uint_t c1 : range(xps)) {
                    if (cpx.empty()) break;

                    vec1d icpx = cpx, icpy = cpy;
                    cpx.clear(); cpy.clear();

                    // Find out if all the current polygon's points are "inside" the
                    // projection's current edge
                    vec1b in(icpx.dims);
                    for (uint_t i : range(icpx)) {
                        in.safe[i] = regrid_drizzle_in_poly_edge(orient,
                            xps.safe[c1], yps.safe[c1], xps.safe[c2], yps.safe[c2],
                            icpx.safe[i], icpy.safe[i]
                        );
                    }

                    uint_t i2 = icpx.size()-1;
                    for (uint_t i1 : range(icpx)) {
                        if (in.safe[i2] != in.safe[i1]) {
                            // This edge [i2-i1] is intersected by the projection's
                            // current edge, find the intersection point and add it
                            // to the polygon
                            double tx, ty;
                            bool intersect = regrid_drizzle_segment_intersect(
                                xps.safe[c1], yps.safe[c1], xps.safe[c2], yps.safe[c2],
                                icpx.safe[i1], icpy.safe[i1], icpx.safe[i2], icpy.safe[i2],
                                tx, ty);

                            if (intersect) {
                                cpx.push_back(tx);
                                cpy.push_back(ty);
                            }
                        }

                        if (in.safe[i1]) {
                            // The point i1 is "inside" the projection's current edge,
                            // keep it for now
                            cpx.push_back(icpx.safe[i1]);
                            cpy.push_back(icpy.safe[i1]);
                        }

                        i2 = i1;
                    }

                    c2 = c1;
                }

                // No intersection, just discard that pixel
                if (cpx.size() < 3) continue;

                // First data entering in this pixel, initialize
                covered = true;

                // Compute the area of this intersection (1: full coverage, 0: no coverage)
                flx += imgs.safe(ipy,ipx)*polyon_area(cpx, cpy);
            }

            return covered;
        }

        template<typename T, typename U>
        bool regrid_nearest(const vec<2,T>& imgs, const vec1d& xps, const vec1d& yps, U& flx) {
            int_t mx = round(mean(xps)), my = round(mean(yps));
            if (mx > 0 && uint_t(mx) < imgs.dims[1] && my > 0 && uint_t(my) < imgs.dims[0]) {
                flx = imgs.safe(uint_t(my), uint_t(mx));
                return true;
            } else {
                return false;
            }
        }

        template<typename T, typename U>
        bool regrid_nearest_fcon(const vec<2,T>& imgs, const vec1d& xps, const vec1d& yps, U& flx) {
            bool covered = regrid_nearest(imgs, xps, yps, flx);
            if (covered) {
                flx *= polyon_area(xps, yps);
            }

            return covered;
        }
    }
}

namespace astro {

    enum class regrid_method {
        drizzle, nearest
    };

    struct regrid_params {
        bool verbose = false;
        bool conserve_flux = false;
        regrid_method method = regrid_method::drizzle;
    };

    template<typename T = double>
    vec2d regrid(const vec<2,T>& imgs, const astro::wcs& astros, const astro::wcs& astrod,
        regrid_params opts = regrid_params{}) {

        // Regridded image
        vec2d res;

#ifdef NO_WCSLIB
        static_assert(!std::is_same<T,T>::value, "WCS support is disabled, "
            "please enable the WCSLib library to use this function");
#else
        res = replicate(dnan, astrod.dims[astrod.y_axis], astrod.dims[astrod.x_axis]);

        // Precompute the projection of the new pixel grid on the old
        // Note: for the horizontal pixel 'i' of line 'j' (x_i,y_j), the grid is:
        //   (pux,puy)[i]     (pux,puy)[i+1]    # y_(j+0.5)
        //   (plx,ply)[i]     (plx,ply)[i+1]    # y_(j-0.5)
        //   # x_(i-0.5)      # x_(i+0.5)
        // To avoid re-computing stuff, pux and puy are moved into plx and ply
        // on each 'y' iteration for reuse.
        auto pg = progress_start(res.size());
        vec1d plx(res.dims[1]+1);
        vec1d ply(res.dims[1]+1);
        for (uint_t ix : range(res.dims[1]+1)) {
            double tra, tdec;
            astro::xy2ad(astrod, ix+0.5, 0.5, tra, tdec);
            astro::ad2xy(astros, tra, tdec, plx.safe[ix], ply.safe[ix]);
            plx.safe[ix] -= 1.0; ply.safe[ix] -= 1.0;
        }

        for (uint_t iy : range(res.dims[0])) {
            vec1d pux(res.dims[1]+1);
            vec1d puy(res.dims[1]+1);
            for (uint_t ix : range(res.dims[1]+1)) {
                double tra, tdec;
                astro::xy2ad(astrod, ix+0.5, iy+1.5, tra, tdec);
                astro::ad2xy(astros, tra, tdec, pux.safe[ix], puy.safe[ix]);
                pux.safe[ix] -= 1.0; puy.safe[ix] -= 1.0;
            }

            for (uint_t ix : range(res.dims[1])) {
                // Find projection of each pixel of the new grid on the original image
                // NB: assumes the astrometry is such that this projection is
                // reasonably approximated by a 4-edge polygon (i.e.: varying pixel scales,
                // pixel offsets and rotations are fine, but weird things may happen close
                // to the poles of the projection where things become non-linear)

                vec1d xps = {plx.safe[ix], plx.safe[ix+1], pux.safe[ix+1], pux.safe[ix]};
                vec1d yps = {ply.safe[ix], ply.safe[ix+1], puy.safe[ix+1], puy.safe[ix]};

                double flx = 0.0;
                bool covered = false;

                switch (opts.method) {
                case regrid_method::drizzle:
                    covered = impl::wcs_impl::regrid_drizzle(imgs, xps, yps, flx);
                    break;
                case regrid_method::nearest:
                    if (opts.conserve_flux) {
                        covered = impl::wcs_impl::regrid_nearest_fcon(imgs, xps, yps, flx);
                    } else {
                        covered = impl::wcs_impl::regrid_nearest(imgs, xps, yps, flx);
                    }
                    break;
                }

                if (covered) {
                    res.safe(iy,ix) = flx;
                }

                if (opts.verbose) progress(pg, 31);
            }

            plx = std::move(pux);
            ply = std::move(puy);
        }
#endif

        return res;
    }
}
}

#endif

