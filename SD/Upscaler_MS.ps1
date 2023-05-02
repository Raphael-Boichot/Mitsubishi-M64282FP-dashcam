$sourcePath = "MS"
$targetPath = "Upscaled_MS"
$ratio = 8

if (!(Test-Path -PathType container $targetPath)) {
  New-Item -ItemType Directory -Path $targetPath
}

Get-ChildItem -Path $sourcePath -Filter *.raw -Recurse | % {
  Add-Type -AssemblyName System.Drawing

  $length = $_.Length
  $bytes = [System.IO.File]::ReadAllBytes($_.FullName)

  $subFolder = "$($pwd)\$($targetPath)\$($_.BaseName)"
  if (!(Test-Path -PathType container $subFolder)) {
    New-Item -ItemType Directory -Path $subFolder
  }
  
  $imageIndex = 0
  $expectPixel = 0 # When $expectPixel is 0, we expect the "RAWDAT" marker
  $targetFileName = ""

  for ($position = 0; $position -lt $length; $position++) {
    if (($expectPixel -eq 0) -And ($position + 6 -lt $length)) {
      $text = [System.Text.Encoding]::ASCII.GetString($bytes, $position, 6)
      if ($text -eq "RAWDAT") {
        $width = $bytes[$position + 6]
        $height = $bytes[$position + 7]
        $registers = $bytes[($position + 8)..($position + 15)] #not yet in use

        # Write-Host "$($text) found at position $($position) ($($width)x$($height))"

        $position += 15
        $expectPixel = $width * $height
        $pixelIndex = 0

        $targetFileName = "$($subFolder)\$(([string]$imageIndex).PadLeft(7, "0")).png"
        $newImage = [System.Drawing.Bitmap]::new($width * $ratio, $height * $ratio)
        $newImage.SetResolution($width * $ratio, $height * $ratio)
        $graphics = [System.Drawing.Graphics]::FromImage($newImage)
        $graphics.Clear([System.Drawing.Color]::Black)

        $imageIndex++
      }
    } elseif ($position -lt $length) {
      $grey = $bytes[$position];
      if ($grey -gt 0) {
        $color = [System.Drawing.Color]::FromArgb($grey, $grey, $grey)
        $brush = [System.Drawing.SolidBrush]::new($color)
        $x = [int][Math]::Floor($pixelIndex % $width)
        $y = [int][Math]::Floor($pixelIndex / $width)

        $graphics.FillRectangle($brush, $x * $ratio, $y * $ratio, $ratio, $ratio)
        $brush.Dispose();
      }

      $expectPixel--
      $pixelIndex++

      # All pixel read. Save image now
      if ($expectPixel -eq 0) {
        $newImage.Save($targetFileName, [System.Drawing.Imaging.ImageFormat]::Png)
        Write-Host "$(([int](($position/$length)*10000))/100)% - Written file '$($targetFileName)'"
      }
    }
  }
  
  $newImage.Dispose()
}  
