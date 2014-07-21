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

#include "pib-encoding.hpp"
#include "pib-common.hpp"
#include "encoding/block-helpers.hpp"
#include "encoding/encoding-buffer.hpp"
#include <vector>
#include <string>

namespace ndn {
namespace pib {

using std::vector;
using std::string;

PibIdentity::PibIdentity()
{
}

PibIdentity::PibIdentity(const Name& identity)
  : m_identity(identity)
{
}

template<bool T>
size_t
PibIdentity::wireEncode(EncodingImpl<T>& block) const
{
  size_t totalLength = m_identity.wireEncode(block);
  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(tlv::pib::Identity);

  return totalLength;
}

template size_t
PibIdentity::wireEncode<true>(EncodingImpl<true>& block) const;

template size_t
PibIdentity::wireEncode<false>(EncodingImpl<false>& block) const;

const Block&
PibIdentity::wireEncode() const
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
PibIdentity::wireDecode(const Block& wire)
{
  if (!wire.hasWire()) {
    throw Tlv::Error("The supplied block does not contain wire format");
  }

  if (wire.type() != tlv::pib::Identity)
    throw Tlv::Error("Unexpected TLV type when decoding PibIdentity");

  m_wire = wire;
  m_identity.wireDecode(m_wire.blockFromValue());
}



PibPublicKey::PibPublicKey()
  : m_isValueSet(false)
{
}

PibPublicKey::PibPublicKey(const Name& keyName, const PublicKey& key)
  : m_isValueSet(true)
  , m_keyName(keyName)
  , m_key(key)
{
}

const Name&
PibPublicKey::getKeyName() const
{
  if (m_isValueSet)
    return m_keyName;
  else
    throw Tlv::Error("PibPublicKey::getKeyName: keyName is not set");
}

const PublicKey&
PibPublicKey::getPublicKey() const
{
  if (m_isValueSet)
    return m_key;
  else
    throw Tlv::Error("PibPublicKey::getPublicKey: key is not set");
}

template<bool T>
size_t
PibPublicKey::wireEncode(EncodingImpl<T>& block) const
{
  size_t totalLength = prependByteArrayBlock(block, tlv::pib::Bytes,
                                             m_key.get().buf(), m_key.get().size());
  totalLength += m_keyName.wireEncode(block);
  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(tlv::pib::PublicKey);

  return totalLength;
}

template size_t
PibPublicKey::wireEncode<true>(EncodingImpl<true>& block) const;

template size_t
PibPublicKey::wireEncode<false>(EncodingImpl<false>& block) const;

const Block&
PibPublicKey::wireEncode() const
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
PibPublicKey::wireDecode(const Block& wire)
{
  if (!wire.hasWire()) {
    throw Tlv::Error("The supplied block does not contain wire format");
  }

  if (wire.type() != tlv::pib::PublicKey)
    throw Tlv::Error("Unexpected TLV type when decoding PibPublicKey");

  m_wire = wire;
  m_wire.parse();

  Block::element_const_iterator it = m_wire.elements_begin();
  if (it != m_wire.elements_end() && it->type() == Tlv::Name) {
    m_keyName.wireDecode(*it);
    it++;
  }
  else
    throw Tlv::Error("PibPublicKey requires the first sub-TLV to be Name");

  if (it != m_wire.elements_end() && it->type() == tlv::pib::Bytes) {
    using namespace CryptoPP;

    StringSource source(it->value(), it->value_size(), true);
    m_key.decode(source);
    it++;
  }
  else
    throw Tlv::Error("PibPublicKey requires the second sub-TLV to be Bytes");

  m_isValueSet = true;
  if (it != m_wire.elements_end())
    throw Tlv::Error("PibPublicKey must contain only two sub-TLVs");
}


PibCertificate::PibCertificate()
  : m_isValueSet(false)
{
}

PibCertificate::PibCertificate(const IdentityCertificate& certificate)
  : m_isValueSet(true)
  , m_certificate(certificate)
{
}

const IdentityCertificate&
PibCertificate::getCertificate() const
{
  if (m_isValueSet)
    return m_certificate;
  else
    throw Tlv::Error("PibCertificate::getCertificate: certificate is not set");
}

template<bool T>
size_t
PibCertificate::wireEncode(EncodingImpl<T>& block) const
{
  size_t totalLength = prependBlock(block, m_certificate.wireEncode());
  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(tlv::pib::Certificate);

  return totalLength;
}

template size_t
PibCertificate::wireEncode<true>(EncodingImpl<true>& block) const;

template size_t
PibCertificate::wireEncode<false>(EncodingImpl<false>& block) const;

const Block&
PibCertificate::wireEncode() const
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
PibCertificate::wireDecode(const Block& wire)
{
  if (!wire.hasWire()) {
    throw Tlv::Error("The supplied block does not contain wire format");
  }

  if (wire.type() != tlv::pib::Certificate)
    throw Tlv::Error("Unexpected TLV type when decoding PibCertificate");

  m_wire = wire;
  m_certificate.wireDecode(m_wire.blockFromValue());

  m_isValueSet = true;
}


PibNameList::PibNameList()
{
}

PibNameList::PibNameList(const std::vector<Name>& names)
  : m_names(names)
{
}

template<bool T>
size_t
PibNameList::wireEncode(EncodingImpl<T>& block) const
{
  size_t totalLength = 0;

  for (vector<Name>::const_reverse_iterator it = m_names.rbegin();
       it != m_names.rend(); it++) {
    totalLength += it->wireEncode(block);
  }

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(tlv::pib::NameList);
  return totalLength;
}

template size_t
PibNameList::wireEncode<true>(EncodingImpl<true>& block) const;

template size_t
PibNameList::wireEncode<false>(EncodingImpl<false>& block) const;


const Block&
PibNameList::wireEncode() const
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
PibNameList::wireDecode(const Block& wire)
{
  if (!wire.hasWire()) {
    throw Tlv::Error("The supplied block does not contain wire format");
  }

  if (wire.type() != tlv::pib::NameList)
    throw Tlv::Error("Unexpected TLV type when decoding PibNameList");

  m_wire = wire;
  m_wire.parse();
  for (Block::element_const_iterator it = m_wire.elements_begin();
       it != m_wire.elements_end(); it++) {
    if (it->type() == Tlv::Name) {
      Name name;
      name.wireDecode(*it);
      m_names.push_back(name);
    }
  }
}

PibError::PibError()
  : m_errCode(ERR_SUCCESS)
{
}

PibError::PibError(uint32_t errCode, const std::string& msg)
  : m_errCode(errCode)
  , m_msg(msg)
{
}

template<bool T>
size_t
PibError::wireEncode(EncodingImpl<T>& block) const
{
  size_t totalLength = 0;
  totalLength += prependByteArrayBlock(block, tlv::pib::Bytes,
                                       reinterpret_cast<const uint8_t*>(m_msg.c_str()),
                                       m_msg.size());
  totalLength += prependNonNegativeIntegerBlock(block, tlv::pib::ErrorCode, m_errCode);
  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(tlv::pib::Error);
  return totalLength;
}

template size_t
PibError::wireEncode<true>(EncodingImpl<true>& block) const;

template size_t
PibError::wireEncode<false>(EncodingImpl<false>& block) const;


const Block&
PibError::wireEncode() const
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
PibError::wireDecode(const Block& wire)
{
  if (!wire.hasWire()) {
    throw Tlv::Error("The supplied block does not contain wire format");
  }

  if (wire.type() != tlv::pib::Error)
    throw Tlv::Error("Unexpected TLV type when decoding Error");

  m_wire = wire;
  m_wire.parse();
  Block::element_const_iterator it = m_wire.elements_begin();

  if (it != m_wire.elements_end() && it->type() == tlv::pib::ErrorCode) {
    m_errCode = readNonNegativeInteger(*it);
    it++;
  }
  else
    throw Tlv::Error("PibError requires the first sub-TLV to be ErrorCode");

  if (it != m_wire.elements_end() && it->type() == tlv::pib::Bytes) {
    m_msg = string(reinterpret_cast<const char*>(it->value()), it->value_size());
    it++;
  }
  else
    throw Tlv::Error("PibError requires the second sub-TLV to be Bytes");
}

PibUser::PibUser()
{
}

void
PibUser::setMgmtCert(const IdentityCertificate& mgmtCert)
{
  m_wire.reset();
  m_mgmtCert = mgmtCert;
}

template<bool T>
size_t
PibUser::wireEncode(EncodingImpl<T>& block) const
{
  size_t totalLength = prependBlock(block, m_mgmtCert.wireEncode());
  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(tlv::pib::User);

  return totalLength;
}

template size_t
PibUser::wireEncode<true>(EncodingImpl<true>& block) const;

template size_t
PibUser::wireEncode<false>(EncodingImpl<false>& block) const;


const Block&
PibUser::wireEncode() const
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
PibUser::wireDecode(const Block& wire)
{
  if (!wire.hasWire()) {
    throw Tlv::Error("The supplied block does not contain wire format");
  }

  if (wire.type() != tlv::pib::User)
    throw Tlv::Error("Unexpected TLV type when decoding Content");

  m_wire = wire;
  m_wire.parse();

  Block::element_const_iterator it = m_wire.elements_begin();
  if (it != m_wire.elements_end() && it->type() == Tlv::Data) {
    m_mgmtCert.wireDecode(*it);
    it++;
  }
  else
    throw Tlv::Error("PibError requires the first sub-TLV to be Data");
}

} // namespace pib
} // namespace ndn
