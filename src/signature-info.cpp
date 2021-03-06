/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2014 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#include "signature-info.hpp"
#include "encoding/block-helpers.hpp"
#include "util/concepts.hpp"

#include <boost/lexical_cast.hpp>

namespace ndn {

BOOST_CONCEPT_ASSERT((boost::EqualityComparable<SignatureInfo>));
BOOST_CONCEPT_ASSERT((WireEncodable<SignatureInfo>));
BOOST_CONCEPT_ASSERT((WireDecodable<SignatureInfo>));
static_assert(std::is_base_of<tlv::Error, SignatureInfo::Error>::value,
              "SignatureInfo::Error must inherit from tlv::Error");

SignatureInfo::SignatureInfo()
  : m_type(-1)
  , m_hasKeyLocator(false)
{
}

SignatureInfo::SignatureInfo(tlv::SignatureTypeValue type)
  : m_type(type)
  , m_hasKeyLocator(false)
{
}

SignatureInfo::SignatureInfo(tlv::SignatureTypeValue type, const KeyLocator& keyLocator)
  : m_type(type)
  , m_hasKeyLocator(true)
  , m_keyLocator(keyLocator)
{
}

SignatureInfo::SignatureInfo(const Block& block)
{
  wireDecode(block);
}

void
SignatureInfo::setSignatureType(tlv::SignatureTypeValue type)
{
  m_wire.reset();
  m_type = type;
}

void
SignatureInfo::setKeyLocator(const KeyLocator& keyLocator)
{
  m_wire.reset();
  m_keyLocator = keyLocator;
  m_hasKeyLocator = true;
}

void
SignatureInfo::unsetKeyLocator()
{
  m_wire.reset();
  m_keyLocator = KeyLocator();
  m_hasKeyLocator = false;
}

const KeyLocator&
SignatureInfo::getKeyLocator() const
{
  if (m_hasKeyLocator)
    return m_keyLocator;
  else
    throw Error("KeyLocator does not exist");
}

void
SignatureInfo::appendTypeSpecificTlv(const Block& block)
{
  m_otherTlvs.push_back(block);
}


const Block&
SignatureInfo::getTypeSpecificTlv(uint32_t type) const
{
  for (std::list<Block>::const_iterator i = m_otherTlvs.begin();
       i != m_otherTlvs.end(); i++) {
    if (i->type() == type)
      return *i;
  }

  throw Error("(SignatureInfo::getTypeSpecificTlv) Requested a non-existed type [" +
              boost::lexical_cast<std::string>(type) + "] from SignatureInfo");
}

template<bool T>
size_t
SignatureInfo::wireEncode(EncodingImpl<T>& block) const
{
  size_t totalLength = 0;

  for (std::list<Block>::const_reverse_iterator i = m_otherTlvs.rbegin();
       i != m_otherTlvs.rend(); i++) {
    totalLength += block.appendBlock(*i);
  }

  if (m_hasKeyLocator)
    totalLength += m_keyLocator.wireEncode(block);

  totalLength += prependNonNegativeIntegerBlock(block, tlv::SignatureType, m_type);

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(tlv::SignatureInfo);
  return totalLength;
}

template size_t
SignatureInfo::wireEncode<true>(EncodingImpl<true>& block) const;

template size_t
SignatureInfo::wireEncode<false>(EncodingImpl<false>& block) const;


const Block&
SignatureInfo::wireEncode() const
{
  if (m_wire.hasWire())
    return m_wire;

  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();
  return m_wire;
}

void
SignatureInfo::wireDecode(const Block& wire)
{
  if (!wire.hasWire()) {
    throw Error("The supplied block does not contain wire format");
  }

  m_hasKeyLocator = false;

  m_wire = wire;
  m_wire.parse();

  if (m_wire.type() != tlv::SignatureInfo)
    throw tlv::Error("Unexpected TLV type when decoding Name");

  Block::element_const_iterator it = m_wire.elements_begin();

  // the first block must be SignatureType
  if (it != m_wire.elements_end() && it->type() == tlv::SignatureType) {
    m_type = readNonNegativeInteger(*it);
    it++;
  }
  else
    throw Error("SignatureInfo does not have sub-TLV or the first sub-TLV is not SignatureType");

  // the second block could be KeyLocator
  if (it != m_wire.elements_end() && it->type() == tlv::KeyLocator) {
    m_keyLocator.wireDecode(*it);
    m_hasKeyLocator = true;
    it++;
  }

  // Decode the rest of type-specific TLVs, if any
  while (it != m_wire.elements_end()) {
    appendTypeSpecificTlv(*it);
    it++;
  }
}

bool
SignatureInfo::operator==(const SignatureInfo& rhs) const
{
  return (m_type == rhs.m_type &&
          m_hasKeyLocator == rhs.m_hasKeyLocator &&
          m_keyLocator == rhs.m_keyLocator &&
          m_otherTlvs == rhs.m_otherTlvs);
}

} // namespace ndn
