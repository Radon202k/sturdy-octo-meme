
internal void
drawDtHistoryGraph(v2 p, f32 height)
{
    f32 maxDt = 0;
    for (u32 dtIndex = 0;
         dtIndex < arrayCount(app.dtHistory);
         ++dtIndex)
    {
        f32 dtValue = app.dtHistory[dtIndex];
        if (dtValue > maxDt)
        {
            maxDt = dtValue;
        }
    }
    
    for (u32 dtIndex = 0;
         dtIndex < arrayCount(app.dtHistory);
         ++dtIndex)
    {
        f32 dtValue = app.dtHistory[dtIndex];
        
        f32 relDt = (dtValue / maxDt);
        
        f32 barHeight = relDt * height;
        f32 barWidth = 2;
        f32 margin = 1;
        
        v2 barDim = 
        {
            barWidth,
            barHeight
        };
        
        v3 barP =
        {
            dtIndex * (barWidth + margin),
            height - barHeight,
        };
        
        v4 color = hnGOLD;
        if (maxDt > 0.018f)
        {
            if (relDt > 0.5f)
            {
                color = hnCRIMSON;
                
                char dtLabel[256] = {};
                hnMakeFloatLabel(dtLabel,sizeof(dtLabel),dtValue);
                
                u32 debugLine = 0;
                
                hnPushStringIndexed(&app.renderer->font, &app.renderer->textureArray, &
                                    app.renderer->ortho2d.vb, &app.renderer->ortho2d.ib,
                                    dtLabel, toV3(p,0) + barP, hnCRIMSON, 0, false, 1);
                
            }
            else if (relDt > 0.25f)
            {
                color = hnYELLOW;
            }
        }
        
        hnPushSpriteIndexed(&app.renderer->ortho2d.vb, &app.renderer->ortho2d.ib, 
                            app.renderer->white, toV3(p,0) + barP, barDim, color, 0, false);
    }
    
}

internal void
updateDebugDisplay(void)
{
    f32 dt = app.renderer->backend->dt;
    
    char dtLabel[256] = {};
    hnMakeFloatLabel(dtLabel,sizeof(dtLabel),dt,"Delta time");
    
    u32 debugLine = 0;
    
    hnPushStringIndexed(&app.renderer->font, &app.renderer->textureArray, &app.renderer->ortho2d.vb, &app.renderer->ortho2d.ib,
                        dtLabel, {0,(f32)(++debugLine)*16}, hnGOLD, 0, false, 1);
    
    char cameraPosLabel[256] = {};
    hnMakeV3Label(cameraPosLabel,sizeof(cameraPosLabel),app.cam.pos,"Camera world pos");
    
    hnPushStringIndexed(&app.renderer->font, &app.renderer->textureArray, &app.renderer->ortho2d.vb, &app.renderer->ortho2d.ib,
                        cameraPosLabel, {0,(f32)(++debugLine)*16}, hnGOLD, 0, false, 1);
    
    drawDtHistoryGraph({0,win32.clientDim.y-100},100);
    
    app.dtHistory[app.dtHistoryIndex++] = app.renderer->backend->dt;
    if (app.dtHistoryIndex == arrayCount(app.dtHistory))
    {
        app.dtHistoryIndex = 0;
    }
    
    app.time += app.renderer->backend->dt;
}