#include "FloatCalculate.h"
#include <stdbool.h>

const size_t SIGN_BIT = 1;
const size_t EXPONENT_BITS = 8;
const size_t MANTISSA_BITS = 23;


// You can also design a function of your own.
static void build_bitstring(Float input, char *output)
{
  output[0] = input.sign + '0';
  for (int i = EXPONENT_BITS - 1; i >= 0; --i)
  {
    output[EXPONENT_BITS - i] = (input.exponent >> i) & 1u ? '1' : '0';
  }
  for (int i = MANTISSA_BITS - 1; i >= 0; --i)
  {
    output[EXPONENT_BITS + MANTISSA_BITS - i] = (input.mantissa >> i) & 1u ? '1' : '0';
  }
  output[EXPONENT_BITS + MANTISSA_BITS + 1] = '\0';
}

// You can also design a function of your own.
static Float parse_bitstring(const char *input)
{
  Float result;
  result.sign = input[0] - '0';
  result.exponent = 0;
  result.mantissa = 0;
  for (size_t i = 1; i <= EXPONENT_BITS; ++i)
  {
    result.exponent = (result.exponent << 1) + (input[i] - '0');
  }
  for (size_t i = EXPONENT_BITS + 1; i <= EXPONENT_BITS + MANTISSA_BITS; ++i)
  {
    result.mantissa = (result.mantissa << 1) + (input[i] - '0');
  }
  if (result.exponent == 0)
  {
    if (result.mantissa == 0)
    {
      result.type = ZERO_T;
    }
    else
    {
      result.type = DENORMALIZED_T;
    }
  }
  else if (result.exponent == (1u << EXPONENT_BITS) - 1)
  {
    if (result.mantissa == 0)
    {
      result.type = result.sign == 0 ? INFINITY_T : -INFINITY_T;
    }
    else
    {
      result.type = NAN_T;
    }
  }
  else
  {
    result.type = NORMALIZED_T;
  }
  return result;
}

// You can also design a function of your own.
static Float float_add_impl(Float a, Float b)
{
  uint32_t a_exponent = a.exponent;
  uint32_t b_exponent = b.exponent;

  if (a.type == DENORMALIZED_T)
  {
    a_exponent += 1;
    a.exponent += 1;
  }

  if (b.type == DENORMALIZED_T)
  {
    b_exponent += 1;
    b.exponent += 1;
  }

  // Normalize the mantissas
  if (a.type == NORMALIZED_T)
  {
    a.mantissa = (1u << (MANTISSA_BITS)) | a.mantissa;
  }
  a.mantissa <<= 3; // Shift to add guard, round, and sticky bits

  if (b.type == NORMALIZED_T)
  {
    b.mantissa = (1u << (MANTISSA_BITS)) | b.mantissa;
  }
  b.mantissa <<= 3;

  if (a_exponent < b_exponent || (a_exponent == b_exponent && a.mantissa < b.mantissa))
  {
    Float temp = a;
    a = b;
    b = temp;
    uint32_t temp_exponent = a_exponent;
    a_exponent = b_exponent;
    b_exponent = temp_exponent;
  }

  if (a_exponent != b_exponent)
  {
    int shift = a_exponent - b_exponent;
    uint32_t grs = (b.mantissa & ((1 << shift) - 1)); // Get the last shift bits
    if (shift >= 32)
    {
      b.mantissa = 0;
    }
    else{
      b.mantissa >>= shift;
    }
    if (grs != 0)
    {
      b.mantissa |= 1; // Set the sticky bit if any of the shifted bits were non-zero
    }

    b.exponent = a_exponent;
  }

  if (a.sign == b.sign)
  {
    a.mantissa += b.mantissa;
  }
  else
  {
    a.mantissa -= b.mantissa;
  }

  if (a.type == NORMALIZED_T)
  {
    if (a.mantissa >= (1u << (MANTISSA_BITS + 3 + 1)))
    {
      while (a.mantissa >= (1u << (MANTISSA_BITS + 3 + 1)))
      {
        uint32_t grs = a.mantissa & 7;
        a.mantissa >>= 1;
        a.exponent += 1;

        if (grs & 4) // sticky
          a.mantissa |= 1;
      }
    }
    else
    {
      while (a.mantissa < (1u << (MANTISSA_BITS + 3)) && a.exponent > 0)
      {
        a.mantissa <<= 1;
        a.exponent -= 1;
      }
      if (a.exponent == 0)
      {
        a.type = DENORMALIZED_T;
        a.exponent += 1;
        a.mantissa >>= 1;
      }
    }
  }
  else {
    if (a.mantissa >= (1u << (MANTISSA_BITS + 3)))
    {
      while (a.mantissa > (1u << (MANTISSA_BITS + 3)))
      {
        uint32_t grs = a.mantissa & 7;
        a.mantissa >>= 1;
        a.exponent += 1;

        if (grs & 4) // sticky
          a.mantissa |= 1;
      }
      a.type = NORMALIZED_T;
      a.mantissa <<= 1;
      a.exponent -= 1;
    }
    else
    {
      while (a.mantissa < (1u << (MANTISSA_BITS + 2)) && a.exponent > 1)
      {
        a.mantissa <<= 1;
        a.exponent -= 1;
      }
    }
  }

  a.mantissa >>= 3;

  if (a.type == DENORMALIZED_T)
  {
    a.exponent -= 1;
  }

  if (a.type == NORMALIZED_T)
  {
    a.mantissa &= ((1u << MANTISSA_BITS) - 1);
  }

  if (a.mantissa == 0 && a.exponent == 0)
  {
    return (Float){a.sign, 0, 0, ZERO_T};
  }

  return (Float){a.sign, a.exponent, a.mantissa, a.type};

}

// You should not modify the signature of this function
void float_add(const char *a, const char *b, char *result)
{
  // TODO: Implement this function
  // A possible implementation of the function:
  Float fa = parse_bitstring(a);
  Float fb = parse_bitstring(b);
  Float fresult = float_add_impl(fa, fb);
  build_bitstring(fresult, result);
}
