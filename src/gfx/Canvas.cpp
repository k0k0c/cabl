/*----------------------------------------------------------------------------------------------------------------------

                 %%%%%%%%%%%%%%%%%
                 %%%%%%%%%%%%%%%%%
                 %%%           %%%
                 %%%           %%%
                 %%%           %%%
%%%%%%%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% www.shaduzlabs.com %%%%%

------------------------------------------------------------------------------------------------------------------------

  Copyright (C) 2014 Vincenzo Pacella

  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this program.
  If not, see <http://www.gnu.org/licenses/>.

----------------------------------------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gfx/Canvas.h"

#include "gfx/Font.h"
#include "gfx/fonts/FontSmall.h"
#include "gfx/fonts/FontNormal.h"
#include "gfx/fonts/FontBig.h"

#include "util/Functions.h"

//----------------------------------------------------------------------------------------------------------------------

#define M_SWAP(a, b) { a ^= b; b ^= a; a ^= b; }

//----------------------------------------------------------------------------------------------------------------------

namespace sl
{
namespace kio
{

//----------------------------------------------------------------------------------------------------------------------

Canvas::Canvas( uint16_t width_, uint16_t height_, tAllocation allocationType_ )
: m_width(width_)
, m_height(height_)
, m_pFont( FontNormal::get() )
{
  switch( allocationType_ )
  {
    case tAllocation::ROW_2BYTES_3_PIXELS:
    {
      m_canvasWidthInBytes = (m_width / 3) * 2;
      m_canvasSizeInBytes = ( m_canvasWidthInBytes * height_ );
      break;
    }
    case tAllocation::COL_1BYTE_8_PIXELS:
    {
      m_canvasWidthInBytes = m_width;
      m_canvasSizeInBytes = ( m_canvasWidthInBytes * ( 1 + ( ( m_height - 1 ) >> 3) ) );
      break;
    }
    case tAllocation::ROW_1BYTE_8_PIXELS:
    default:
    {
      m_canvasWidthInBytes = ( 1 + ( ( m_width - 1 ) >> 3) );
      m_canvasSizeInBytes = ( m_canvasWidthInBytes * height_ );
      break;
    }
  }
  m_data.resize(m_canvasSizeInBytes);
}

//----------------------------------------------------------------------------------------------------------------------


Canvas::~Canvas()
{
 // delete[] m_data;
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::invert()
{
  for( uint16_t i = 0; i < m_canvasSizeInBytes; i++ )
    m_data[i] = ~m_data[ i ];
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::fillPattern( uint8_t value_ )
{
  memset( m_data.data(), value_, m_canvasSizeInBytes );
}
  
//----------------------------------------------------------------------------------------------------------------------

void Canvas::setPixel( uint16_t x_, uint16_t y_, tColor color_ )
{
  if ( x_ >= getWidth() || y_ >= getHeight() || color_ == tColor::NONE )
    return;
  
  if( color_ == tColor::RANDOM )
    color_ = static_cast<tColor>( util::randomRange(0,2) );
  
  uint16_t byteIndex = ( m_canvasWidthInBytes * y_ ) + ( x_ >> 3 );
  
  switch( color_ )
  {
    case tColor::WHITE:
      m_data[ byteIndex ] |= ( 0x80 >> ( x_ & 7 ) );
      break;
      
    case tColor::BLACK:
      m_data[ byteIndex ] &= ( ~0x80 >> ( x_ & 7 ) );
      break;
      
    case tColor::INVERT:
      m_data[ byteIndex ] ^= ( 0x80 >> ( x_ & 7 ) );
      break;
      
    default:
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------

Canvas::tColor Canvas::getPixel(uint16_t x_, uint16_t y_ ) const
{
  if ( x_ >= getWidth() || y_ >= getHeight() )
    return tColor::BLACK;
  
  return
  ( m_data[ ( m_canvasWidthInBytes * y_ ) + ( x_ >> 3 ) ] & ( 0x80 >> ( x_  & 7 ) ) ) == 0
  ? tColor::BLACK
  : tColor::WHITE;
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawLine( uint16_t x0_, uint16_t y0_, uint16_t x1_, uint16_t y1_, tColor color_ )
{
  int32_t e;
  int32_t dx,dy;
  int16_t s1,s2;
  int16_t x,y;
  bool bSwapped = false;
  
  x = x0_;
  y = y0_;
  
  if ( x1_ < x0_ )
  {
    dx = x0_ - x1_;
    s1 = -1;
  }
  else if ( x1_ == x0_ )
  {
    dx = 0;
    s1 = 0;
  }
  else
  {
    dx = x1_ - x0_;
    s1 = 1;
  }
  
  if ( y1_ < y0_ )
  {
    dy = y0_ - y1_;
    s2 = -1;
  }
  else if ( y1_ == y0_ )
  {
    dy = 0;
    s2 = 0;
  }
  else
  {
    dy = y1_ - y0_;
    s2 = 1;
  }
  
  if ( dy > dx )
  {
    M_SWAP(dx,dy);
    bSwapped = true;
  }
  
  e = ( (int)dy << 1 ) - dx;
  
  for ( uint16_t j=0; j <= dx; j++ )
  {
    setPixel( x, y, color_ );
    
    if ( e >= 0 )
    {
      if ( bSwapped )
        x = x + s1;
      else
        y = y + s2;
      e = e - ((int)dx<<1);
    }
    if ( bSwapped )
      y = y + s2;
    else
      x = x + s1;
    e = e + ((int)dy<<1);
  }
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::Canvas::drawLineVertical( uint16_t x_, uint16_t y_, uint16_t l_, tColor color_ )
{
  for( uint16_t y = y_; y<y_+l_; y++ )
    setPixel( x_, y, color_ );
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawLineHorizontal( uint16_t x_, uint16_t y_, uint16_t l_, tColor color_ )
{
  for( uint16_t x = x_; x<x_+l_; x++ )
    setPixel( x, y_, color_ );
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawTriangle(
  uint16_t x0_,
  uint16_t y0_,
  uint16_t x1_,
  uint16_t y1_,
  uint16_t x2_,
  uint16_t y2_,
  tColor color_
)
{
  drawLine( x0_, y0_, x1_, y1_, color_ );
  drawLine( x1_, y1_, x2_, y2_, color_ );
  drawLine( x2_, y2_, x0_, y0_, color_ );
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawFilledTriangle(
  uint16_t x0_,
  uint16_t y0_,
  uint16_t x1_,
  uint16_t y1_,
  uint16_t x2_,
  uint16_t y2_,
  tColor color_,
  tColor fillColor_
)
{
  // Original Author: Adafruit Industries (Adafruit GFX library)
  
  int16_t a, b, y, last;
  
  // Sort coordinates by y order (y2 >= y1 >= y0)
  if (y0_ > y1_) {
    M_SWAP(y0_, y1_);
    M_SWAP(x0_, x1_);
  }
  if (y1_ > y2_) {
    M_SWAP(y2_, y1_);
    M_SWAP(x2_, x1_);
  }
  if (y0_ > y1_) {
    M_SWAP(y0_, y1_);
    M_SWAP(x0_, x1_);
  }
  
  if( y0_ == y2_ ) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0_;
    if( x1_ < a )
      a = x1_;
    else if( x1_ > b )
      b = x1_;
    if( x2_ < a )
      a = x2_;
    else if( x2_ > b )
      b = x2_;
    drawLineHorizontal( a, y0_, b-a+1, color_ );
    return;
  }
  
  int16_t
  dx01 = x1_ - x0_,
  dy01 = y1_ - y0_,
  dx02 = x2_ - x0_,
  dy02 = y2_ - y0_,
  dx12 = x2_ - x1_,
  dy12 = y2_ - y1_;
  int32_t
  sa   = 0,
  sb   = 0;
  
  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1_ == y2_)
    last = y1_;   // Include y1 scanline
  else
    last = y1_-1; // Skip it
  
  for(y=y0_; y<=last; y++)
  {
    a   = x0_ + sa / dy01;
    b   = x0_ + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
     a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
     b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
     */
    if(a > b) M_SWAP(a,b);
    drawLineHorizontal(a, y, b-a+1, color_);
  }
  
  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1_);
  sb = dx02 * (y - y0_);
  for(; y<=y2_; y++)
  {
    a   = x1_ + sa / dy12;
    b   = x0_ + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
     a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
     b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
     */
    if(a > b)
    {
      M_SWAP(a,b);
    }
    drawLineHorizontal( a, y, b-a+1, color_ );
  }
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawRect( uint16_t x_, uint16_t y_, uint16_t w_, uint16_t h_, tColor color_ )
{
  drawFilledRect( x_, y_, w_, h_, color_, tColor::NONE );
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawFilledRect( uint16_t x_, uint16_t y_, uint16_t w_, uint16_t h_, tColor color_, tColor fillColor_ )
{
  if( x_ > getWidth() || y_ > getHeight() || w_ == 0 || h_ == 0 )
    return;
  
  drawLineHorizontal( x_, y_, w_, color_);
  drawLineHorizontal( x_, y_+h_-1, w_, color_);
  drawLineVertical( x_, y_, h_, color_);
  drawLineVertical( x_+w_-1, y_, h_, color_);
  
  if( fillColor_ == tColor::NONE )
    return;
  
  if( w_ > h_ )
  {
    uint16_t lineWidth = w_- 2;
    for ( uint16_t i = y_ + 1; i < y_ + h_ - 1; i++) {
      drawLineHorizontal( x_+1, i, lineWidth, fillColor_ );
    }
  }
  else{
    uint16_t lineHeight = h_ - 2;
    for ( uint16_t i = x_ + 1; i < x_ + w_ - 1; i++) {
      drawLineVertical( i, y_+1, lineHeight, fillColor_ );
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawRectRounded(uint16_t x_, uint16_t y_, uint16_t w_, uint16_t h_, uint16_t r_, tColor color_)
{
  drawFilledRectRounded( x_, y_, w_, h_, r_, color_, tColor::NONE );
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawFilledRectRounded(
  uint16_t x_,
  uint16_t y_,
  uint16_t w_,
  uint16_t h_,
  uint16_t r_,
  tColor color_,
  tColor fillColor_
)
{
  if( x_ > getWidth() || y_ > getHeight() || w_ == 0 || h_ == 0 )
    return;

  uint16_t smallestSide = ( w_ >= h_ ) ? h_ : w_;
  uint16_t rOffset = 2 * r_;
  if( rOffset > smallestSide )
  {
    r_ = smallestSide / 2;
    rOffset = smallestSide;
  }
  drawLineHorizontal( ( x_+r_       ), ( y_          ), ( w_ - rOffset ), color_ );
  drawLineHorizontal( ( x_+r_       ), ( y_ + h_ - 1 ), ( w_ - rOffset ), color_ );
  drawLineVertical  ( ( x_          ), ( y_ + r_     ), ( h_ - rOffset ), color_ );
  drawLineVertical  ( ( x_ + w_ - 1 ), ( y_ + r_     ), ( h_ - rOffset ), color_ );
  
  drawFilledCircle( ( x_ + r_         ), ( y_ + r_         ), r_, color_, fillColor_, tCircle::QUARTER_TOP_LEFT );
  drawFilledCircle( ( x_ + w_ - r_ -1 ), ( y_ + r_         ), r_, color_, fillColor_, tCircle::QUARTER_TOP_RIGHT );
  drawFilledCircle( ( x_ + w_ - r_ -1 ), ( y_ + h_ - r_ -1 ), r_, color_, fillColor_, tCircle::QUARTER_BOTTOM_RIGHT );
  drawFilledCircle( ( x_ + r_         ), ( y_ + h_ - r_ -1 ), r_, color_, fillColor_, tCircle::QUARTER_BOTTOM_LEFT );
  
  if( fillColor_ == tColor::NONE )
    return;
  
  drawFilledRect( ( x_ + r_ ), ( y_ + 1           ), ( w_ - rOffset ), ( r_           ), fillColor_, fillColor_ );
  drawFilledRect( ( x_ + 1  ), ( y_ + r_          ), ( w_ - 2       ), ( h_ - rOffset ), fillColor_, fillColor_ );
  drawFilledRect( ( x_ + r_ ), ( y_ + h_ - 1 - r_ ), ( w_ - rOffset ), ( r_           ), fillColor_, fillColor_ );
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawCircle( uint16_t x_, uint16_t y_, uint16_t r_, tColor color_, tCircle type_ )
{
  drawFilledCircle(x_, y_, r_, color_, tColor::NONE, type_ );
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawFilledCircle(
  uint16_t x_,
  uint16_t y_,
  uint16_t r_,
  tColor color_,
  tColor fillColor_,
  tCircle type_
)
{
  if ( ( x_ >= m_width ) || ( y_ >= m_height ) || r_ == 0 )
    return;

  int x, y, rX0,rX1, rY0, rY1;
  rX0 = rY0 = -r_;
  rX1 = rY1 = r_;
  
  switch( type_ )
  {
    case tCircle::SEMI_LEFT:
      rX1 = 0;
      break;
    case tCircle::SEMI_RIGHT:
      rX0 = 0;
      break;
    case tCircle::SEMI_TOP:
      rY1 = 0;
      break;
    case tCircle::SEMI_BOTTOM:
      rY0 = 0;
      break;
      
    case tCircle::QUARTER_TOP_LEFT:
      rX1 = 0;
      rY1 = 0;
      break;
    case tCircle::QUARTER_TOP_RIGHT:
      rX0 = 0;
      rY1 = 0;
      break;
    case tCircle::QUARTER_BOTTOM_LEFT:
      rX1 = 0;
      rY0 = 0;
      break;
    case tCircle::QUARTER_BOTTOM_RIGHT:
      rX0 = 0;
      rY0 = 0;
      break;
      
    default:
    case tCircle::FULL:
      break;
      
  }
  
  for( x = rX0; x <= rX1; x++ )
  {
    for( y = rY0; y <= rY1; y++ )
    {
      int32_t xysq = ( ( x*x ) + ( y*y ) );
      int32_t rsq = r_*r_;
      if( ( rsq - xysq < r_ ) && ( xysq - rsq < r_ ) )
      {
        setPixel( ( x + x_), ( y + y_ ), color_ );
      }
      else if( fillColor_ != tColor::NONE && ( xysq < rsq ) )
      {
        setPixel( ( x + x_), ( y + y_), fillColor_ );
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::drawBitmap(
  uint16_t x_,
  uint16_t y_,
  uint16_t w_,
  uint16_t h_,
  const uint8_t* pBitmap_,
  tColor color_
)
{
  if ( ( x_ >= m_width ) || ( y_ >= m_height ) )
    return;
  
  uint16_t drawableHeight = ( ( y_ + h_ ) > m_height ) ? ( m_height - y_ ) : h_;
  uint16_t drawableWidth = ( ( x_ + w_ ) > m_width ) ? ( m_width - x_ ) : w_;
  
  for (uint8_t j=0; j<drawableHeight; j++)
  {
    for (uint8_t i=0; i<drawableWidth; i++ )
    {
      if ( pBitmap_[ (i>>3) + j*(w_>>3)] & ( 0x01 << ( 7-(i & 7)) ) )
      {
        setPixel( x_+i, y_+j, color_ );
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::draw(
  const Canvas& c_,
  uint16_t xDest_,
  uint16_t yDest_,
  uint16_t xSource_,
  uint16_t ySource_,
  uint16_t w_,
  uint16_t h_
)
{
  if ( ( xDest_ >= m_width ) || ( yDest_ >= m_height ) || ( xSource_ >= c_.getWidth() ) || ( ySource_ >= c_.getHeight()))
    return;
  
  uint16_t width  = ( w_ <= c_.getWidth()  && w_ > 0 ) ? w_ : c_.getWidth();
  uint16_t height = ( h_ <= c_.getHeight() && h_ > 0 ) ? h_ : c_.getHeight();
  
  uint16_t drawableHeight = ( ( yDest_ + height ) > m_height ) ? ( m_height - yDest_ ) : height;
  uint16_t drawableWidth = ( ( xDest_ + width ) > m_width ) ? ( m_width - xDest_ ) : width;
  
  for (uint8_t j=0; j<drawableHeight; j++)
  {
    for (uint8_t i=0; i<drawableWidth; i++ )
    {
      setPixel( xDest_+i, yDest_+j, c_.getPixel( xSource_ + i, ySource_ + j ) );
    }
  }

}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::printChar(uint16_t x_, uint16_t y_, char c_, Font* pFont_, tColor color_ ) {
  
  uint8_t c = c_ - pFont_->getFirstChar();
  
  if ( ( x_ >= m_width ) || ( y_ >= m_height ) || c > pFont_->getLastChar() || c_ < pFont_->getFirstChar() )
    return;
  
  for ( uint8_t y=0; y < pFont_->getHeight(); y++ )
  {
    for ( uint8_t x=0; x < pFont_->getHeight(); x++ )
    {
      if( pFont_->getPixel( c, x, y ) )
      {
        setPixel( ( x_ + x     ), y_ + y, color_ );
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::printStr(
  uint16_t x_,
  uint16_t y_,
  const char *pStr_,
  tFont font_,
  tColor color_,
  uint8_t spacing_
)
{
  Font* pFont;
  switch( font_ )
  {
    case tFont::SMALL:
      pFont = FontSmall::get();
      break;
    case tFont::NORMAL:
      pFont = FontNormal::get();
      break;
    case tFont::BIG:
      pFont = FontBig::get();
      break;
    case tFont::DEFAULT:
    default:
      pFont = m_pFont;
      break;
  }
  
  printStr( x_, y_, pStr_, pFont, color_, spacing_ );
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::printStr(
  uint16_t x_,
  uint16_t y_,
  const char *pStr_,
  Font* pFont_,
  tColor color_,
  uint8_t spacing_
)
{
  uint8_t charWidth = pFont_->getCharSpacing() + spacing_;
  if (y_ >= m_height || x_ > m_width )
    return;
  for (unsigned i=0; static_cast<unsigned>(pStr_[i]) != 0; i++)
  {
    if ( x_ > ( m_width + charWidth ) )
      return;
    printChar( x_, y_, pStr_[i], pFont_, color_ );
    x_ += charWidth;
  }
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::setDefaultFont( tFont font_ )
{
  switch( font_ )
  {
    case tFont::SMALL:
      m_pFont = FontSmall::get();
      break;
    case tFont::NORMAL:
      m_pFont = FontNormal::get();
      break;
    case tFont::BIG:
      m_pFont = FontBig::get();
      break;
    case tFont::DEFAULT:
    default:
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------

void Canvas::toString()
{
  printf("\n");
  
  printf("+");
  for( int col = 0; col < m_width; col++ )
    printf("-");
  printf("+\n");
  
  for( int row = 0; row < m_height; row++ )
  {
    printf("|");
    for( int col = 0; col < m_width; col++ )
    {
      if( getPixel(col,row) == tColor::BLACK )
        printf(" ");
      else
        printf("0");
    }
    printf("|");
    printf("\n");
  }
  
  printf("+");
  for( int col = 0; col < m_width; col++ )
    printf("-");
  printf("+\n");
}

//----------------------------------------------------------------------------------------------------------------------

} // kio
} // sl
