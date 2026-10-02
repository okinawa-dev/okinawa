#ifndef OK_MATERIAL_HPP
#define OK_MATERIAL_HPP

#include <array>
#include <string>

class OkTexture;

/**
 * @brief How a material's pixels meet what is already drawn behind them.
 *
 * One property with four answers, because the four exclude each other:
 * a surface is solid, or cut out of a quad, or seen through, or adds
 * light. They used to be separate switches on the item, which allowed
 * combinations that mean nothing.
 *
 * In every mode a texture's alpha is coverage and nothing else: how much
 * of the pixel the material covers. The mode says what is done with it.
 */
enum OkBlendMode {
  /// Solid. The alpha takes no part in whether a pixel is drawn.
  OK_BLEND_OPAQUE = 0,
  /// All or nothing: a pixel under half covered is not drawn, the rest
  /// are solid. A grille, a leaf, a railing -- no blending, no sorting.
  OK_BLEND_CUTOUT,
  /// Seen through: the pixel is mixed with what is behind it by its
  /// coverage times the material's opacity. Glass, water, an overlay.
  OK_BLEND_ALPHA,
  /// Adds light to what is behind it. A halo, a glow.
  OK_BLEND_ADDITIVE
};

/**
 * @brief What a surface is drawn with, apart from its texture: how it
 *        blends, whether light models it, and what it is tinted.
 *
 * An item has one, and each range of its faces may have its own -- so a
 * window is one object with a solid frame and a pane seen through, not
 * two objects for one surface.
 *
 * It is plain state. The item that holds it owns the tint mask's
 * reference in the texture cache; a material copied about by a caller
 * holds none.
 */
struct OkMaterial {
  /// How many zones of a tint mask can be tinted on their own.
  static const int SLOTS = 3;

  OkBlendMode blend;
  /// Skip the lights and the scene's tint: a light source, a debug line.
  bool unlit;
  /// Multiplies the texture; its alpha is the material's opacity, which
  /// counts in the two blended modes.
  std::array<float, 4> tint;
  /// The colour each zone of the tint mask takes.
  std::array<std::array<float, 3>, SLOTS> slotTint;
  /// Per zone: 0 multiplies the tint over the texture, 1 keeps only the
  /// texture's luminance and lets the tint set the hue.
  std::array<float, SLOTS> slotLuminance;
  /// The weights of the zones, one per channel of a second texture laid
  /// over the same coordinates; null for none.
  OkTexture  *tintMask;
  std::string tintMaskName;

  OkMaterial() {
    blend    = OK_BLEND_OPAQUE;
    unlit    = false;
    tintMask = nullptr;
    tint.fill(1.0f);
    for (int i = 0; i < SLOTS; i++) {
      slotTint[static_cast<size_t>(i)].fill(1.0f);
      slotLuminance[static_cast<size_t>(i)] = 0.0f;
    }
  }

  /**
   * @brief Whether it is drawn in the late pass, after everything solid.
   *
   * The two blended modes do not write depth, so anything solid drawn
   * after them would paint over them.
   */
  bool isBlended() const {
    return blend == OK_BLEND_ALPHA || blend == OK_BLEND_ADDITIVE;
  }
};

#endif
