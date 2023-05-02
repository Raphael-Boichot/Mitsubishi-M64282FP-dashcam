$sourcePath = "Camera"
$targetPath = "Upscaled_Camera"
$ratio = 8

If (!(Test-Path -PathType container $targetPath)) {
  New-Item -ItemType Directory -Path $targetPath
}

Get-ChildItem -Path $sourcePath -Filter *.bmp -Recurse | % {
  Add-Type -AssemblyName system.drawing
  $imageFormat = "System.Drawing.Imaging.ImageFormat" -as [type]

  $targetFileName = "$($pwd)\$($targetPath)\$($_.BaseName).png"
  $image = [Drawing.Image]::FromFile($_.FullName)

  $targetWidth = [int] ($image.Width * $ratio)
  $targetHeight = [int] ($image.Height * $ratio)

  $newImage = [System.Drawing.Bitmap]::new($targetWidth, $targetHeight)
  $newImage.SetResolution($targetWidth, $targetHeight)

  $graphics = [System.Drawing.Graphics]::FromImage($newImage)
  $graphics.Clear([System.Drawing.Color]::Black)
  
  $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor

  # Draw all pixels as filled squares, because [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor seems to be broken when upscaling
  for ($x = 0; $x -lt $image.Width; $x++) {
    for ($y = 0; $y -lt $image.Height; $y++) {
      $color = $image.GetPixel($x, $y)
      $brush = [System.Drawing.SolidBrush]::new($color)
      $graphics.FillRectangle($brush, $x * $ratio, $y * $ratio, $ratio, $ratio)
      $brush.Dispose()
    }
  }

  Write-Host "Converting '$($_.FullName)' -> '$($targetFileName)'"

  $newImage.Save($targetFileName, $imageFormat::Png)

  $image.Dispose()
  $newImage.Dispose()
}  
